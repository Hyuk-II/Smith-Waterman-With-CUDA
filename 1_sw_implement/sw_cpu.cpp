#include "functions.h"
#include "sequence_codec.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

void smith_waterman_cpu(vector<string> sequences);

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

    // 추출된 문자열로 score table 구성
    smith_waterman_cpu(sequences);

    return 0;
}

// smith waterman CPU 연산
void smith_waterman_cpu(vector<string> sequences) {
    SequenceCodec codec;
    vector<int> seq1_int = codec.encode(sequences[0]);
    vector<int> seq2_int = codec.encode(sequences[1]);

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

            int match_mis_score = (seq1_int[i - 1] == seq2_int[j - 1])
                                      ? MATCH_SCORE
                                      : MISMATCH_SCORE;
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

    cout << "=== 연산 완료 ===" << endl;
    cout << "최대 정렬 점수 : " << max_score << endl;
    cout << "최대 점수 좌표 : (" << max_i << ", " << max_j << ")" << endl;

    // 연산이 끝나면 시작좌표(max_i, max_j)부터 역추적 함수 호출
    run_traceback(score_table, sequences[0], sequences[1], seq1_int, seq2_int,
                  cols, max_i, max_j);
}