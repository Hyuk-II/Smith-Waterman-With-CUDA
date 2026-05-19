#include "functions.h"
#include "sequence_codec.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<int> smith_waterman_cpu(vector<string> sequences);

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
    vector<int> score_table = smith_waterman_cpu(sequences);

    return 0;
}

// smith waterman CPU 연산
vector<int> smith_waterman_cpu(vector<string> sequences) {
    // 1. 서열 인코딩 (String -> int 배열)
    SequenceCodec codec;
    vector<int> seq1_int = codec.encode(sequences[0]);
    vector<int> seq2_int = codec.encode(sequences[1]);

    int len1 = seq1_int.size(); // 행 (Rows)
    int len2 = seq2_int.size(); // 열 (Cols)

    int rows = len1 + 1;
    int cols = len2 + 1;

    // DP 테이블 생성
    vector<int> dp_table(rows * cols, 0);

    // 인덱싱 람다 함수 (인라인 처리되어 오버헤드 없음)
    auto get_idx = [&](int r, int c) { return r * cols + c; };

    // 기본 스코어링 파라미터
    // (추후 BLOSUM62 같은 2차원 점수 매트릭스로 교체)
    const int MATCH_SCORE = 3;
    const int MISMATCH_SCORE = -3;
    const int GAP_PENALTY = -2;

    int max_score = 0; // 전체 테이블에서 가장 높은 점수 추적

    cout << "\n=== DP 테이블 연산 시작 ===" << endl;
    cout << "테이블 크기: " << rows << " x " << cols << " ("
         << (rows * cols * 4) / 1024 << " KB)" << endl;

    // DP 테이블 순회 및 연산
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {

            // 대각선 (Match or Mismatch)
            int match_mis_score = (seq1_int[i - 1] == seq2_int[j - 1])
                                      ? MATCH_SCORE
                                      : MISMATCH_SCORE;
            int diag_score = dp_table[get_idx(i - 1, j - 1)] + match_mis_score;

            // 위에서 내려옴 (Insert Gap)
            int up_score = dp_table[get_idx(i - 1, j)] + GAP_PENALTY;

            // 왼쪽에서 넘어옴 (Delete Gap)
            int left_score = dp_table[get_idx(i, j - 1)] + GAP_PENALTY;

            // 점수 취합 및 Clipping (0 이하 점수는 버림)
            // C++ initializer list를 사용해 4개의 값 중 최대값을 한 번에 도출
            int current_score = max({0, diag_score, up_score, left_score});

            // 테이블에 결과 저장
            dp_table[get_idx(i, j)] = current_score;

            // 글로벌 최대 점수 갱신
            if (current_score > max_score) {
                max_score = current_score;
            }
        }
    }

    cout << "=== 연산 완료 ===" << endl;
    cout << "최대 정렬 점수 : " << max_score << endl;

    return dp_table;
}