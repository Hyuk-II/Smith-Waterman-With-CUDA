#include "common.h"
#include "sequence_codec.h"

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// 타일 32x32 크기
#define TILE_SIZE 32

__constant__ int d_BLOSUM62[20][20];

// GPU 공유 메모리 타일링 커널
__global__ void sw_kernel_tiled(int*, const uint8_t*, const uint8_t*, int, int, int, int, int);

// GPU 스미스-워터맨 파이프라인
SWResult smith_waterman_gpu(const vector<uint8_t>& seq1_int, const vector<uint8_t>& seq2_int);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "[사용법] ./sw_gpu_tiled <서열파일명1> <서열파일명2>" << endl;
        cerr << "예시: ./sw_gpu_tiled example_seq1.txt example_seq2.txt" << endl;
        return 1;
    }

    // Context Initialization 패널티를 벤치마크 밖으로 빼기 위한 웜업
    cudaFree(0); 

    vector<string> sequences = get_sequences(argv);
    if (sequences.size() == 0) return 1;

    SequenceCodec codec;
    vector<uint8_t> seq1_int = codec.encode(sequences[0]);
    vector<uint8_t> seq2_int = codec.encode(sequences[1]);
    
    auto start_dp = high_resolution_clock::now();
    SWResult result = smith_waterman_gpu(seq1_int, seq2_int);
    auto end_dp = high_resolution_clock::now();

    run_traceback(result, sequences[0], sequences[1], seq1_int, seq2_int);
    
    duration<double, std::milli> dp_ms = end_dp - start_dp;

    double gcups =
        ((double)seq1_int.size() * seq2_int.size()) / (dp_ms.count() * 1e6);

    cout << "\n========================================" << endl;
    cout << " [ Tiled GPU 벤치마크 결과 요약 ]" << endl;
    cout << "----------------------------------------" << endl;
    cout << " 최대 정렬 점수\t\t: " << result.max_score << endl;
    cout << " 점수 테이블 연산\t: " << dp_ms.count() << " ms" << endl;
    cout << " GCUPS\t\t\t: " << fixed << setprecision(3) << gcups << endl;
    cout << "========================================\n" << endl;
    
    return 0;
}

// GPU 공유 메모리 타일링 커널
__global__ void sw_kernel_tiled(int* score_table, const uint8_t* seq1, const uint8_t* seq2, 
                                int len1, int len2, int cols, 
                                int block_diag_idx, int num_blocks_row) {
    
    // 현재 블록이 맡을 타일의 2D 좌표 (block_i, block_j) 계산
    int b = blockIdx.x;
    int block_i, block_j;
    if (block_diag_idx < num_blocks_row) {
        block_i = block_diag_idx - b;
        block_j = b;
    } else {
        block_i = (num_blocks_row - 1) - b;
        block_j = (block_diag_idx - num_blocks_row + 1) + b;
    }

    int tx = threadIdx.x; // 0 ~ 31

    // 공유 메모리 할당
    __shared__ uint8_t seq1_s[TILE_SIZE];
    __shared__ uint8_t seq2_s[TILE_SIZE];
    __shared__ int H_s[TILE_SIZE + 1][TILE_SIZE + 1];

    // 타일의 실제 글로벌 시작 좌표 (1-based index)
    int start_i = block_i * TILE_SIZE + 1;
    int start_j = block_j * TILE_SIZE + 1;

    // 글로벌 메모리 -> 공유 메모리로 적재
    if (start_i + tx <= len1) seq1_s[tx] = seq1[start_i + tx - 1]; 
    else seq1_s[tx] = 20; // 'X' 처리

    if (start_j + tx <= len2) seq2_s[tx] = seq2[start_j + tx - 1]; 
    else seq2_s[tx] = 20;

    // DP 의존성을 위한 위쪽 테두리와 왼쪽 테두리 가져오기
    if (start_i > 1 && start_j + tx <= len2)
        H_s[0][tx + 1] = score_table[(start_i - 1) * cols + (start_j + tx)];
    else
        H_s[0][tx + 1] = 0;

    if (start_j > 1 && start_i + tx <= len1)
        H_s[tx + 1][0] = score_table[(start_i + tx) * cols + (start_j - 1)];
    else
        H_s[tx + 1][0] = 0;

    // 타일의 좌상단 꼭짓점 가져오기
    if (tx == 0) {
        if (start_i > 1 && start_j > 1) H_s[0][0] = score_table[(start_i - 1) * cols + (start_j - 1)];
        else H_s[0][0] = 0;
    }

    // 모든 스레드가 공유 메모리 적재를 완료할 때까지 대기
    __syncthreads();

    // 타일 내부 Wavefront 연산
    for (int d = 0; d < 2 * TILE_SIZE - 1; d++) {
        int i = min(d, TILE_SIZE - 1) - tx + 1;
        int j = max(0, d - TILE_SIZE + 1) + tx + 1;

        if (i >= 1 && i <= TILE_SIZE && j >= 1 && j <= TILE_SIZE) {
            if (start_i + i - 1 <= len1 && start_j + j - 1 <= len2) {
                int idx1 = seq1_s[i - 1];
                int idx2 = seq2_s[j - 1];
                int match_mis_score = (idx1 < 20 && idx2 < 20) ? d_BLOSUM62[idx1][idx2] : -4;

                int diag = H_s[i - 1][j - 1] + match_mis_score;
                int up = H_s[i - 1][j] + GAP_PENALTY;
                int left = H_s[i][j - 1] + GAP_PENALTY;

                H_s[i][j] = max(0, max(diag, max(up, left)));
            } else {
                H_s[i][j] = 0;
            }
        }
        // 각 내부 대각선이 끝날 때마다 동기화
        __syncthreads();
    }

    // 공유 메모리 -> 글로벌 메모리로 연산 결과 기록
    // 각 스레드가 타일의 한 행을 통째로 글로벌 테이블에 복사
    if (start_i + tx <= len1) {
        for (int curr_j = 1; curr_j <= TILE_SIZE; curr_j++) {
            if (start_j + curr_j - 1 <= len2) {
                score_table[(start_i + tx) * cols + (start_j + curr_j - 1)] = H_s[tx + 1][curr_j];
            }
        }
    }
}

SWResult smith_waterman_gpu(const vector<uint8_t>& seq1_int, const vector<uint8_t>& seq2_int) {
    int len1 = seq1_int.size();
    int len2 = seq2_int.size();
    int rows = len1 + 1;
    int cols = len2 + 1;
    int table_size = rows * cols;

    vector<int> h_score_table(table_size, 0);
    int *d_score_table;
    uint8_t *d_seq1, *d_seq2;
    CUDA_CHECK(cudaMalloc(&d_score_table, (size_t)table_size * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_seq1, len1 * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_seq2, len2 * sizeof(uint8_t)));

    CUDA_CHECK(cudaMemcpy(d_score_table, h_score_table.data(), (size_t)table_size * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_seq1, seq1_int.data(), len1 * sizeof(uint8_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_seq2, seq2_int.data(), len2 * sizeof(uint8_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpyToSymbol(d_BLOSUM62, BLOSUM62, 20 * 20 * sizeof(int)));
    

    // 블록 단위로 쪼갠 전체 그리드 크기 계산
    int num_blocks_row = (len1 + TILE_SIZE - 1) / TILE_SIZE;
    int num_blocks_col = (len2 + TILE_SIZE - 1) / TILE_SIZE;
    int num_block_diagonals = num_blocks_row + num_blocks_col - 1;

    // Macro Wavefront 커널 실행, 타일 대각선 개수만큼만 루프
    for (int bd = 0; bd < num_block_diagonals; bd++) {
        int num_blocks_in_diag = min(bd + 1, min(num_blocks_row, num_blocks_col));
        if (bd >= num_blocks_row) {
            num_blocks_in_diag = num_blocks_row + num_blocks_col - bd - 1;
        }

        // 스레드 개수는 타일 크기와 동일하게 매핑
        sw_kernel_tiled<<<num_blocks_in_diag, TILE_SIZE>>>(d_score_table, d_seq1, d_seq2, len1, len2, cols, bd, num_blocks_row);
    }
    
    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaMemcpy(h_score_table.data(), d_score_table, (size_t)table_size * sizeof(int), cudaMemcpyDeviceToHost));

    cudaFree(d_score_table);
    cudaFree(d_seq1);
    cudaFree(d_seq2);

    int max_score = 0;
    int max_i = 0;
    int max_j = 0;
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            if (h_score_table[i * cols + j] > max_score) {
                max_score = h_score_table[i * cols + j];
                max_i = i;
                max_j = j;
            }
        }
    }

    return {h_score_table, max_score, max_i, max_j};
}