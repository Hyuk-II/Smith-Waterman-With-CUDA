#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;

// 개별 FASTA 파일을 파싱하여 output 폴더에 저장
void processFastaFile(const fs::path &input_filepath,
                      const fs::path &output_dir) {
    ifstream infile(input_filepath);
    if (!infile.is_open()) {
        cerr << "[에러] 파일을 열 수 없습니다: " << input_filepath << endl;
        return;
    }

    // FASTA 내부의 염기서열 문자열 추출
    // >로 시작하는 헤더찾아, 다음 헤더 or 파일의 끝까지 존재하는 염기서열추출
    vector<string> sequences;
    string current_seq = "";
    string line;

    while (getline(infile, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;

        if (line[0] == '>') {
            if (!current_seq.empty()) {
                sequences.push_back(current_seq);
                current_seq = "";
            }
        } else {
            current_seq += line;
        }
    }
    if (!current_seq.empty())
        sequences.push_back(current_seq);
    infile.close();

    // 원본 파일명을 기반으로 출력 파일명 생성 (예: data.fasta -> data_seq1.txt)
    string base_filename = input_filepath.stem().string();

    int size = sequences.size();
    for (int i = 0; i < size; i++) {
        // 번호가 증가하는 파일명 동적 생성
        string seq_filename =
            base_filename + "_seq" + to_string(i + 1) + ".txt";
        fs::path out_path = output_dir / seq_filename;

        ofstream out(out_path);
        if (!out.is_open()) {
            cerr << "[경고] 파일을 생성할 수 없습니다: " << out_path << endl;
            continue;
        }

        out << sequences[i];
        out.close();
    }

    cout << "[완료] " << input_filepath.filename() << " -> " << size
         << "개의 서열 추출 완료" << endl;
}

int main() {
    // 입출력 디렉토리 경로 설정
    fs::path input_dir = "input_fasta";
    fs::path output_dir = "output_sequence";

    //  Input 디렉토리 없으면 에러 종료
    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        cerr << "[에러] '" << input_dir
             << "' 폴더가 존재하지 않습니다. 폴더를 생성하고 FASTA 파일을 "
                "넣어주세요."
             << endl;
        return 1;
    }

    // Output 디렉토리 없으면 자동으로 생성
    if (!fs::exists(output_dir)) {
        fs::create_directory(output_dir);
        cout << "[알림] '" << output_dir << "' 폴더를 새로 생성했습니다."
             << endl;
    }

    cout << "=== FASTA 전처리 파이프라인 시작 ===" << endl;

    int processed_count = 0;

    // Input 디렉토리 내의 모든 파일 순회
    for (const auto &entry : fs::directory_iterator(input_dir)) {
        // .fasta 또는 .fa 확장자인 파일만 처리
        if (entry.is_regular_file()) {
            string ext = entry.path().extension().string();
            if (ext == ".fasta" || ext == ".fa") {
                processFastaFile(entry.path(), output_dir);
                processed_count++;
            }
        }
    }

    cout << "=== 전처리 작업 종료 (총 " << processed_count
         << "개 파일 처리됨) ===" << endl;

    return 0;
}