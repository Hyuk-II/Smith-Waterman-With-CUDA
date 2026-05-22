#pragma once

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

class SequenceCodec {
  private:
    uint8_t ascii_2_index[128]; // 아스키코드 매핑 테이블
    char index_2_ascii[22];     // index 매핑 테이블

  public:
    SequenceCodec() {
        for (int i = 0; i < 128; i++) {
            // 테이블 초기화
            ascii_2_index[i] = 20; // 20 안쓰는 값
        }

        for (int i = 0; i < 22; i++) {
            // 테이블 초기화
            index_2_ascii[i] = 'X'; // X 안쓰는 값
        }

        // 양방향 매핑
        string amino_acids = "ARNDCQEGHILKMFPSTWYV";
        for (int i = 0; i < 20; i++) {
            char aa = amino_acids[i];

            ascii_2_index[aa] = i; // [인코딩용] A -> 0
            index_2_ascii[i] = aa; // [디코딩용] 0 -> A
        }

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

    string decode(vector<uint8_t> encode_seq) {
        int len = encode_seq.size();
        string decode_seq = "";

        decode_seq.reserve(len);

        for (int i = 0; i < len; i++) {
            int idx = encode_seq[i];

            if (idx > 21) {
                idx = 20;
            }
            decode_seq += index_2_ascii[idx];
        }

        return decode_seq;
    }
};