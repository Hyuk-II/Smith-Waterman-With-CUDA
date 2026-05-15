#include "sequence_codec.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<string> getSequences(char *argv[]);

int main(int argc, char *argv[]) {
    // 명령줄 인수 개수 검증
    if (argc != 3) {
        cerr << "[사용법] ./sw_cpu <서열파일명1> <서열파일명2>" << endl;
        cerr << "예시: ./sw_cpu example_seq1.txt example_seq2.txt" << endl;
        return 1;
    }

    vector<string> sequences = getSequences(argv);
    if (sequences.size() == 0) {
        return 1;
    }

    SequenceCodec codec = SequenceCodec();
    vector<int> seq1_int = codec.encode(sequences[0]);
    vector<int> seq2_int = codec.encode(sequences[1]);

    for (int i = 0; i < seq1_int.size(); i++) {
        cout << seq1_int[i] << " ";
    }
    cout << endl;

    for (int i = 0; i < seq2_int.size(); i++) {
        cout << seq2_int[i] << " ";
    }

    cout << codec.decode(seq1_int) << endl;
    cout << codec.decode(seq2_int) << endl;

    return 0;
}

vector<string> getSequences(char *argv[]) {
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