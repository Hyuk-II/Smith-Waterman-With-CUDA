#include "common.h"
#include "sequence_codec.h"

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

// GPU 커널용 상수 메모리 선언
__constant__ int d_BLOSUM62[20][20];

// GPU Wavefront 커널, 대각선 병렬 계산
__global__ void sw_kernel_wavefront(int*, const uint8_t*, const uint8_t*, int, int, int, int);

// GPU 스미스-워터맨 파이프라인
SWResult smith_waterman_gpu(const vector<uint8_t>&, const vector<uint8_t>&);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "[사용법] ./sw_gpu <서열파일명1> <서열파일명2>" << endl;
        cerr << "예시: ./sw_gpu example_seq1.txt example_seq2.txt" << endl;
        return 1;
    }

    cudaFree(0);

    vector<string> sequences = get_sequences(argv);
    if (sequences.size() == 0) return 1;

    SequenceCodec codec;
    vector<uint8_t> seq1_int = codec.encode(sequences[0]);
    vector<uint8_t> seq2_int = codec.encode(sequences[1]);
    
    // 추출된 문자열로 gpu기반 smith waterman 알고리즘 수행
    // 구조체로 테이블과 최대 좌표 리턴
    auto start_dp = high_resolution_clock::now();
    SWResult result = smith_waterman_gpu(seq1_int, seq2_int);
    auto end_dp = high_resolution_clock::now();

    // 최장 유사 문자열 출력
    run_traceback(result, sequences[0], sequences[1], seq1_int, seq2_int);

    // H2D + Kernel + D2H 전체 포함 시간 계산 
    duration<double, std::milli> dp_ms = end_dp - start_dp;

    cout << "\n========================================" << endl;
    cout << " [ GPU 벤치마크 결과 요약 ]" << endl;
    cout << "----------------------------------------" << endl;
    cout << " 최대 정렬 점수 : " << result.max_score << endl;
    cout << " 점수 테이블 연산 : " << dp_ms.count() << " ms" << endl;
    cout << "========================================\n" << endl;
    
    return 0;
}

__global__ void sw_kernel_wavefront(int* score_table, const uint8_t* seq1, const uint8_t* seq2, 
                                    int len1, int len2, int cols, int d) {
    // 현재 스레드의 글로벌 인덱스
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    // 대각선 d에 대한 시작 좌표 계산
    int start_i = min(d, len1);
    int start_j = max(1, d - len1 + 1);

    // 현재 스레드가 담당할 실제 테이블 좌표 계산
    int i = start_i - tid;
    int j = start_j + tid;

    // 테이블 유효 범위 내의 스레드만 연산 수행
    if (i >= 1 && j >= 1 && i <= len1 && j <= len2) {
        int idx1 = seq1[i - 1];
        int idx2 = seq2[j - 1];
        
        // __constant__ 메모리에서 BLOSUM62 참조
        int match_mis_score = (idx1 < 20 && idx2 < 20) ? d_BLOSUM62[idx1][idx2] : -4;

        // DP 점수 계산 (GAP_PENALTY는 -2로 고정)
        int diag_score = score_table[(i - 1) * cols + (j - 1)] + match_mis_score;
        int up_score   = score_table[(i - 1) * cols + j] + GAP_PENALTY;
        int left_score = score_table[i * cols + (j - 1)] + GAP_PENALTY;

        // CUDA 내장 함수로 사용한 최대값 도출 (Clipping to 0)
        int current_score = 0;
        current_score = max(current_score, diag_score);
        current_score = max(current_score, up_score);
        current_score = max(current_score, left_score);

        // 글로벌 메모리에 결과 기록
        score_table[i * cols + j] = current_score;
    }
}

SWResult smith_waterman_gpu(const vector<uint8_t>& seq1_int, const vector<uint8_t>& seq2_int) {
    int len1 = seq1_int.size();
    int len2 = seq2_int.size();
    int rows = len1 + 1;
    int cols = len2 + 1;
    int table_size = rows * cols;

    // Host 메모리 할당
    vector<int> h_score_table(table_size, 0);

    // Device 메모리 할당
    int *d_score_table;
    uint8_t *d_seq1, *d_seq2;
    cudaMalloc(&d_score_table, table_size * sizeof(int));
    cudaMalloc(&d_seq1, len1 * sizeof(uint8_t));
    cudaMalloc(&d_seq2, len2 * sizeof(uint8_t));

    // 데이터 Host -> Device 복사
    cudaMemcpy(d_score_table, h_score_table.data(), table_size * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_seq1, seq1_int.data(), len1 * sizeof(uint8_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_seq2, seq2_int.data(), len2 * sizeof(uint8_t), cudaMemcpyHostToDevice);
    
    // BLOSUM62 상수 메모리 복사
    cudaMemcpyToSymbol(d_BLOSUM62, BLOSUM62, 20 * 20 * sizeof(int));

    // Wavefront 커널 실행, 대각선 개수만큼 루프
    int num_diagonals = len1 + len2 - 1;
    int threadsPerBlock = 256;

    cout << "\n=== [GPU] 테이블 연산 시작 ===" << endl;
    cout << "총 대각선(Kernel Launch) 횟수: " << num_diagonals << endl;

    for (int d = 1; d <= num_diagonals; d++) {
        // 현재 대각선에 포함된 칸의 개수 계산
        int num_elements_in_diag = min(d, min(len1, len2));
        if (d > max(len1, len2)) {
            num_elements_in_diag = len1 + len2 - d + 1;
        }

        // 스레드 블록 수 동적 할당
        int blocksPerGrid = (num_elements_in_diag + threadsPerBlock - 1) / threadsPerBlock;
        
        sw_kernel_wavefront<<<blocksPerGrid, threadsPerBlock>>>(d_score_table, d_seq1, d_seq2, len1, len2, cols, d);
    }
    
    // GPU 작업 완료 대기
    cudaDeviceSynchronize();

    // 데이터 Device -> Host 복사
    cudaMemcpy(h_score_table.data(), d_score_table, table_size * sizeof(int), cudaMemcpyDeviceToHost);

    // Device 메모리 해제
    cudaFree(d_score_table);
    cudaFree(d_seq1);
    cudaFree(d_seq2);

    // Max 점수 및 좌표 탐색
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