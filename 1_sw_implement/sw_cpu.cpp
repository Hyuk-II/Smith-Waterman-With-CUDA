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

    vector<string> sequences = get_sequences(argv);
    if (sequences.size() == 0) {
        return 1;
    }

    return 0;
}

// vector<int> smith_waterman_cpu(vector<string> sequences) {}