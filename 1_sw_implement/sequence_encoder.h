#pragma once

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

class SequenceEncoder {
  private:
    int ascii_2_index[128]; // 아스키코드 매핑 테이블

  public:
    SequenceEncoder() {
        for (int i = 0; i < 128; i++) {
            // 테이블 초기화
            ascii_2_index[i] = 20; // 20 안쓰는 값
        }

        // 단백질 서열 매핑
        ascii_2_index['A'] = 0;
        ascii_2_index['R'] = 1;
        ascii_2_index['N'] = 2;
        ascii_2_index['D'] = 3;
        ascii_2_index['C'] = 4;
        ascii_2_index['Q'] = 5;
        ascii_2_index['E'] = 6;
        ascii_2_index['G'] = 7;
        ascii_2_index['H'] = 8;
        ascii_2_index['I'] = 9;
        ascii_2_index['L'] = 10;
        ascii_2_index['K'] = 11;
        ascii_2_index['M'] = 12;
        ascii_2_index['F'] = 13;
        ascii_2_index['P'] = 14;
        ascii_2_index['S'] = 15;
        ascii_2_index['T'] = 16;
        ascii_2_index['W'] = 17;
        ascii_2_index['Y'] = 18;
        ascii_2_index['V'] = 19;

        // Gap 매핑
        ascii_2_index['-'] = 21;
    }

    vector<uint8_t> encode(string &sequence) {
        int len = sequence.length();
        vector<uint8_t> encode_seq(len);

        for (int i = 0; i < len; i++) {
            encode_seq[i] = ascii_2_index[sequence[i]]; // 문자 배열 정수로 매핑
        }
        return encode_seq;
    }
};