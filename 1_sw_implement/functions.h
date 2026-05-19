#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
const int MATCH_SCORE = 3;
const int MISMATCH_SCORE = -3;
const int GAP_PENALTY = -2;

vector<string> get_sequences(char *argv[]) {
    vector<string> sequences;

    // 상대 경로 디렉토리 설정
    const string BASE_DIR = "../0_preprocessing/output_sequence/";

    // 디렉토리 경로와 입력받은 파일명을 결합하여 최종 경로 생성
    string file1_path = BASE_DIR + argv[1];
    string file2_path = BASE_DIR + argv[2];

    cout << "=== 데이터 로드 시작 ===" << endl;
    cout << "Target 1: " << file1_path << endl;
    cout << "Target 2: " << file2_path << endl;

    // 첫 번째 서열 파일 읽기
    ifstream in1(file1_path);
    if (!in1.is_open()) {
        cerr << "[에러] 파일을 열 수 없습니다. 파일명을 다시 확인하세요: "
             << file1_path << endl;
        return sequences;
    }
    string seq1;
    in1 >> seq1;
    in1.close();

    // 두 번째 서열 파일 읽기
    ifstream in2(file2_path);
    if (!in2.is_open()) {
        cerr << "[에러] 파일을 열 수 없습니다. 파일명을 다시 확인하세요: "
             << file2_path << endl;
        return sequences;
    }
    string seq2;
    in2 >> seq2;
    in2.close();

    // 로드된 데이터 검증 출력
    cout << "\n=== 데이터 로드 완료 ===" << endl;

    // 첫 번째 서열 출력
    cout << "[Seq 1] 길이: " << seq1.length() << " AA" << endl;
    if (seq1.length() > 50) {
        cout << "내용(Preview): " << seq1.substr(0, 50) << "..." << endl;
    } else {
        cout << "내용: " << seq1 << endl;
    }

    // 두 번째 서열 출력
    cout << "\n[Seq 2] 길이: " << seq2.length() << " AA" << endl;
    if (seq2.length() > 50) {
        cout << "내용(Preview): " << seq2.substr(0, 50) << "..." << endl;
    } else {
        cout << "내용: " << seq2 << endl;
    }

    sequences.push_back(seq1);
    sequences.push_back(seq2);

    return sequences;
}

// 2개의 문자열의 score_table에서, 가장 유사한 문자열을 추적하여 출력하는 함수
void run_traceback(const vector<int> &score_table, const string &seq1,
                   const string &seq2, const vector<int> &seq1_int,
                   const vector<int> &seq2_int, int cols, int max_i,
                   int max_j) {
    cout << "\n=== 트레이스백 (Traceback) 시작 ===" << endl;

    // DP 테이블 1차원 인덱싱 람다 (트레이스백 전용)
    auto get_idx = [&](int r, int c) { return r * cols + c; };

    string align1 = "";
    string align2 = "";
    string match_line = "";

    int curr_i = max_i;
    int curr_j = max_j;

    // 경로 추적, 현재 최댓값인 점수가, 대각선, 위, 왼쪽, 어디에서 온것인지
    // 확인하고 해당하는 문자로 문자열 구성
    while (curr_i > 0 && curr_j > 0 &&
           score_table[get_idx(curr_i, curr_j)] > 0) {

        int current_score = score_table[get_idx(curr_i, curr_j)];
        int match_mis_score = (seq1_int[curr_i - 1] == seq2_int[curr_j - 1])
                                  ? MATCH_SCORE
                                  : MISMATCH_SCORE;

        // 대각선 (Match/Mismatch)
        if (current_score ==
            score_table[get_idx(curr_i - 1, curr_j - 1)] + match_mis_score) {
            align1 += seq1[curr_i - 1];
            align2 += seq2[curr_j - 1];

            if (seq1_int[curr_i - 1] == seq2_int[curr_j - 1]) {
                match_line += "|";
            } else {
                match_line += " ";
            }
            curr_i--;
            curr_j--;
        }
        // 2. 위 (Insert)
        else if (current_score ==
                 score_table[get_idx(curr_i - 1, curr_j)] + GAP_PENALTY) {
            align1 += seq1[curr_i - 1];
            align2 += '-';
            match_line += " ";
            curr_i--;
        }
        // 3. 왼쪽 (Delete)
        else if (current_score ==
                 score_table[get_idx(curr_i, curr_j - 1)] + GAP_PENALTY) {
            align1 += '-';
            align2 += seq2[curr_j - 1];
            match_line += " ";
            curr_j--;
        }
    }

    reverse(align1.begin(), align1.end());
    reverse(align2.begin(), align2.end());
    reverse(match_line.begin(), match_line.end());

    cout << "\n[최적 로컬 정렬 결과]" << endl;
    cout << "Seq 1: " << align1 << endl;
    cout << "Match: " << match_line << endl;
    cout << "Seq 2: " << align2 << endl;
    cout << "정렬 길이: " << align1.length() << " AA\n" << endl;
}