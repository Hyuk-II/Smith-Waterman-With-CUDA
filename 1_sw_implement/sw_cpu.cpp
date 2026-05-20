#include "common.h"
#include "sequence_codec.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace std::chrono;

SWResult smith_waterman_cpu(const vector<int> &seq1_int,
                            const vector<int> &seq2_int);

int main(int argc, char *argv[]) {
    // 명령줄 인수 개수 검증
    if (argc != 3) {
        cerr << "[사용법] ./sw_cpu <서열파일명1> <서열파일명2>" << endl;
        cerr << "예시: ./sw_cpu example_seq1.txt example_seq2.txt" << endl;
        return 1;
    }

    // 2개의 단백질 서열 추출
    vector<string> sequences = get_sequences(argv);
    if (sequences.size() == 0) {
        return 1;
    }

    SequenceCodec codec;
    vector<int> seq1_int = codec.encode(sequences[0]);
    vector<int> seq2_int = codec.encode(sequences[1]);

    // 추출된 문자열로 cpu기반 smith waterman 알고리즘 수행
    auto start_dp = high_resolution_clock::now();
    // 구조체로 테이블과 최대 좌표 리턴
    SWResult result = smith_waterman_cpu(seq1_int, seq2_int);
    auto end_dp = high_resolution_clock::now();

    // 최장 유사 문자열 출력
    auto start_tb = high_resolution_clock::now();
    run_traceback(result, sequences[0], sequences[1], seq1_int, seq2_int);
    auto end_tb = high_resolution_clock::now();

    duration<double, std::milli> dp_ms = end_dp - start_dp;
    duration<double, std::milli> tb_ms = end_tb - start_tb;

    cout << "\n===  CPU 성능 측정 결과 ===" << endl;
    cout << "- DP 테이블 생성 : " << dp_ms.count() << " ms" << endl;
    cout << "- 트레이스백 연산: " << tb_ms.count() << " ms" << endl;
    cout << "- Total 소요 시간: " << (dp_ms + tb_ms).count() << " ms" << endl;
    cout << "============================\n" << endl;

    return 0;
}

// smith waterman CPU 연산
SWResult smith_waterman_cpu(const vector<int> &seq1_int,
                            const vector<int> &seq2_int) {

    int len1 = seq1_int.size();
    int len2 = seq2_int.size();
    int rows = len1 + 1;
    int cols = len2 + 1;

    vector<int> score_table(rows * cols, 0);
    auto get_idx = [&](int r, int c) { return r * cols + c; };

    int max_score = 0;
    int max_i = 0;
    int max_j = 0;

    cout << "\n=== DP 테이블 연산 시작 ===" << endl;
    cout << "테이블 크기: " << rows << " x " << cols << " ("
         << (rows * cols * 4) / 1024 << " KB)" << endl;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {

            int idx1 = seq1_int[i - 1];
            int idx2 = seq2_int[j - 1];

            // 분기문 없이 배열 직접 참조, O(1)
            // 'X'(알 수 없는 문자)나 20 이상의 잘못된 값이 들어오면 -4점 부여

            int match_mis_score =
                (idx1 < 20 && idx2 < 20) ? BLOSUM62[idx1][idx2] : -4;

            int diag_score =
                score_table[get_idx(i - 1, j - 1)] + match_mis_score;
            int up_score = score_table[get_idx(i - 1, j)] + GAP_PENALTY;
            int left_score = score_table[get_idx(i, j - 1)] + GAP_PENALTY;

            int current_score = max({0, diag_score, up_score, left_score});
            score_table[get_idx(i, j)] = current_score;

            if (current_score > max_score) {
                max_score = current_score;
                max_i = i;
                max_j = j;
            }
        }
    }

    return {score_table, max_score, max_i, max_j};
}