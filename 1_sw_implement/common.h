#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

// smith waterman 연산의 결과
struct SWResult {
    std::vector<int> score_table; // 2개의 문자열에 대한 점수 테이블
    int max_score;                // 가장 높은 점수
    int max_i;                    // 가장 높은 점수 좌표
    int max_j;                    // 가장 높은 점수 좌표
};

const int GAP_PENALTY = -2;

#ifdef __CUDACC__
#define CUDA_CHECK(call)                                                        \
    do {                                                                        \
        cudaError_t _err = (call);                                              \
        if (_err != cudaSuccess) {                                              \
            std::cerr << "[CUDA ERROR] " << cudaGetErrorString(_err)            \
                      << " (" << __FILE__ << ":" << __LINE__ << ")"             \
                      << std::endl;                                             \
            exit(EXIT_FAILURE);                                                 \
        }                                                                       \
    } while (0)
#endif

// SequenceCodec의 "ARNDCQEGHILKMFPSTWYV" 순서에 매핑된 BLOSUM62 행렬
const int BLOSUM62[20][20] = {
    {4, -1, -2, -2, 0, -1, -1, 0, -2, -1, -1, -1, -1, -2, -1, 1, 0, -3, -2, 0},
    {-1, 5, 0, -2, -3, 1, 0, -2, 0, -3, -2, 2, -1, -3, -2, -1, -1, -3, -2, -3},
    {-2, 0, 6, 1, -3, 0, 0, 0, 1, -3, -3, 0, -2, -3, -2, 1, 0, -4, -2, -3},
    {-2, -2, 1, 6, -3, 0, 2, -1, -1, -3, -4, -1, -3, -3, -1, 0, -1, -4, -3, -3},
    {0,  -3, -3, -3, 9,  -3, -4, -3, -3, -1,
     -1, -3, -1, -2, -3, -1, -1, -2, -2, -1},
    {-1, 1, 0, 0, -3, 5, 2, -2, 0, -3, -2, 1, 0, -3, -1, 0, -1, -2, -1, -2},
    {-1, 0, 0, 2, -4, 2, 5, -2, 0, -3, -3, 1, -2, -3, -1, 0, -1, -3, -2, -2},
    {0,  -2, 0,  -1, -3, -2, -2, 6,  -2, -4,
     -4, -2, -3, -3, -2, 0,  -2, -2, -3, -3},
    {-2, 0, 1, -1, -3, 0, 0, -2, 8, -3, -3, -1, -2, -1, -2, -1, -2, -2, 2, -3},
    {-1, -3, -3, -3, -1, -3, -3, -4, -3, 4, 2, -3, 1, 0, -3, -2, -1, -3, -1, 3},
    {-1, -2, -3, -4, -1, -2, -3, -4, -3, 2, 4, -2, 2, 0, -3, -2, -1, -2, -1, 1},
    {-1, 2, 0, -1, -3, 1, 1, -2, -1, -3, -2, 5, -1, -3, -1, 0, -1, -3, -2, -2},
    {-1, -1, -2, -3, -1, 0, -2, -3, -2, 1, 2, -1, 5, 0, -2, -1, -1, -1, -1, 1},
    {-2, -3, -3, -3, -2, -3, -3, -3, -1, 0, 0, -3, 0, 6, -4, -2, -2, 1, 3, -1},
    {-1, -2, -2, -1, -3, -1, -1, -2, -2, -3,
     -3, -1, -2, -4, 7,  -1, -1, -4, -3, -2},
    {1, -1, 1, 0, -1, 0, 0, 0, -1, -2, -2, 0, -1, -2, -1, 4, 1, -3, -2, -2},
    {0, -1, 0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1, -2, -1, 1, 5, -2, -2, 0},
    {-3, -3, -4, -4, -2, -2, -3, -2, -2, -3,
     -2, -3, -1, 1,  -4, -3, -2, 11, 2,  -3},
    {-2, -2, -2, -3, -2, -1, -2, -3, 2, -1,
     -1, -2, -1, 3,  -3, -2, -2, 2,  7, -1},
    {0, -3, -3, -3, -1, -2, -2, -3, -3, 3, 1, -2, 1, -1, -2, -2, 0, -3, -1, 4}};

std::vector<std::string> get_sequences(char *argv[]) {
    std::vector<std::string> sequences;

    // 기본 데이터 위치: 소스 트리에서 1_sw_implement를 CWD로 실행할 때의 fallback
    // 절대 경로나 다른 상대 경로를 인자로 주면 그대로 사용
    const std::string BASE_DIR = "../0_preprocessing/output_sequence/";

    auto resolve_path = [&](const std::string &arg) -> std::string {
        std::ifstream probe(arg);
        if (probe.good()) return arg;   // 주어진 경로가 곧바로 열리면 사용
        return BASE_DIR + arg;          // 실패 시 BASE_DIR 접두사 fallback
    };

    std::string file1_path = resolve_path(argv[1]);
    std::string file2_path = resolve_path(argv[2]);

    std::cout << "=== 데이터 로드 시작 ===" << std::endl;
    std::cout << "Target 1: " << file1_path << std::endl;
    std::cout << "Target 2: " << file2_path << std::endl;

    // 첫 번째 서열 파일 읽기
    std::ifstream in1(file1_path);
    if (!in1.is_open()) {
        std::cerr << "[에러] 파일을 열 수 없습니다. 파일명을 다시 확인하세요: "
                  << file1_path << std::endl;
        return sequences;
    }
    std::string seq1;
    in1 >> seq1;
    in1.close();

    // 두 번째 서열 파일 읽기
    std::ifstream in2(file2_path);
    if (!in2.is_open()) {
        std::cerr << "[에러] 파일을 열 수 없습니다. 파일명을 다시 확인하세요: "
                  << file2_path << std::endl;
        return sequences;
    }
    std::string seq2;
    in2 >> seq2;
    in2.close();

    // 로드된 데이터 검증 출력
    std::cout << "\n=== 데이터 로드 완료 ===" << std::endl;

    // 첫 번째 서열 출력
    std::cout << "[Seq 1] 길이: " << seq1.length() << " AA" << std::endl;
    if (seq1.length() > 50) {
        std::cout << "내용(Preview): " << seq1.substr(0, 50) << "..." << std::endl;
    } else {
        std::cout << "내용: " << seq1 << std::endl;
    }

    // 두 번째 서열 출력
    std::cout << "\n[Seq 2] 길이: " << seq2.length() << " AA" << std::endl;
    if (seq2.length() > 50) {
        std::cout << "내용(Preview): " << seq2.substr(0, 50) << "..." << std::endl;
    } else {
        std::cout << "내용: " << seq2 << std::endl;
    }

    sequences.push_back(seq1);
    sequences.push_back(seq2);

    return sequences;
}

// 2개의 문자열의 score_table에서, 가장 유사한 문자열을 추적하여 출력하는 함수
void run_traceback(const SWResult &result, const std::string &seq1,
                   const std::string &seq2, const std::vector<uint8_t> &seq1_int,
                   const std::vector<uint8_t> &seq2_int) {
    std::cout << "\n=== 트레이스백 (Traceback) 시작 ===" << std::endl;

    // DP 테이블 1차원 인덱싱 람다
    int cols = seq2_int.size() + 1;
    auto get_idx = [&](int r, int c) { return r * cols + c; };

    std::string align1 = "";
    std::string align2 = "";

    int curr_i = result.max_i;
    int curr_j = result.max_j;

    // 경로 추적, 현재 최댓값인 점수가, 대각선, 위, 왼쪽, 어디에서 온것인지
    // 확인하고 해당하는 문자로 문자열 구성
    while (curr_i > 0 && curr_j > 0 &&
           result.score_table[get_idx(curr_i, curr_j)] > 0) {

        int current_score = result.score_table[get_idx(curr_i, curr_j)];
        int idx1 = seq1_int[curr_i - 1];
        int idx2 = seq2_int[curr_j - 1];

        int match_mis_score =
            (idx1 < 20 && idx2 < 20) ? BLOSUM62[idx1][idx2] : -4;

        // 대각선 (Match/Mismatch)
        if (current_score ==
            result.score_table[get_idx(curr_i - 1, curr_j - 1)] +
                match_mis_score) {
            align1 += seq1[curr_i - 1];
            align2 += seq2[curr_j - 1];

            curr_i--;
            curr_j--;
        }
        // 위 (Insert)
        else if (current_score ==
                 result.score_table[get_idx(curr_i - 1, curr_j)] +
                     GAP_PENALTY) {
            align1 += seq1[curr_i - 1];
            align2 += '-';
            curr_i--;
        }
        // 왼쪽 (Delete)
        else if (current_score ==
                 result.score_table[get_idx(curr_i, curr_j - 1)] +
                     GAP_PENALTY) {
            align1 += '-';
            align2 += seq2[curr_j - 1];
            curr_j--;
        }
    }

    std::reverse(align1.begin(), align1.end());
    std::reverse(align2.begin(), align2.end());

    std::cout << "\n[최적 로컬 정렬 결과]" << std::endl;
    std::cout << "Seq 1: " << align1 << std::endl;
    std::cout << "Seq 2: " << align2 << std::endl;
    std::cout << "정렬 길이: " << align2.length() << " AA\n" << std::endl;
}
