## FASTA 데이터 전처리 파이프라인 (Preprocessor)

## 📌 개요

본 모듈은 생물정보학 표준 데이터 포맷인 FASTA 파일(.fasta, .fa)을 파싱하여, 메타데이터를 제거하고 순수한 염기/단백질 서열 문자열만 추출하는 C++ 기반 전처리 파이프라인입니다.

추출된 서열은 이후 진행될 Smith-Waterman GPU/CPU 정렬 알고리즘의 1차원 배열 인코딩을 위한 입력 데이터로 사용됩니다.

## 📂 디렉토리 구조 및 데이터 흐름

입력: input_fasta/

분석하고자 하는 원본 .fasta 또는 .fa 파일들을 위치시킵니다.

출력 : output_sequence/

전처리기가 실행되면 자동으로 생성되는 디렉토리입니다.

추출된 순수 서열들이 개별 .txt 파일로 분리되어 저장됩니다.

## ⚙️ 핵심 동작 로직

메타데이터 분리: > 기호로 시작하는 헤더 라인을 식별하여 건너뛰고, 다음 헤더가 나타나기 전까지의 모든 알파벳 문자열을 단일 서열로 병합합니다.

동적 파일 생성: 하나의 FASTA 파일 안에 여러 개의 서열이 존재할 경우, 식별 번호를 부여하여 개별 텍스트 파일로 완벽하게 분할합니다. (예: data.fasta -> data_seq1.txt, data_seq2.txt)

OS 크로스 플랫폼 대응: Windows 환경에서 작성된 파일의 캐리지 리턴(\r) 찌꺼기를 안전하게 제거하여 문자열 파싱 오류를 원천 차단합니다.

모던 C++ 제어: <filesystem> API를 사용하여 운영체제에 종속되지 않고 안전하게 입출력 디렉토리를 제어 및 순회합니다.

## 🚀 빌드 및 실행 방법

이 코드는 std::filesystem 라이브러리를 사용하므로, 반드시 C++17 표준 이상으로 컴파일해야 합니다.

1. 컴파일 (Compile)

```Bash
g++ -std=c++17 preprocessor.cpp -o preprocessor
```

2. 실행 (Execute)

```Bash
./preprocessor
```

## 실행 예시

[입력] input_fasta/example.fasta

\>sp|P40283|H2B11_ARATH Histone H2B.11 OS=Arabidopsis thaliana OX=3702 GN=At5g59910 PE=1 SV=5
MAPKAEKKPAEKKPASEKPVEEKSKAEKAPAEKKPKAGKKLPKEAGAGGDKKKKMKKKSV
ETYKIYIFKVLKQVHPDIGISSKAMGIMNSFINDIFEKLAQEASKLARYNKKPTITSREI
QTAVRLVLPGELAKHAVSEGTKAVTKFTSS

\>sp|Q9FFC0|H2B10_ARATH Histone H2B.10 OS=Arabidopsis thaliana OX=3702 GN=At5g22880 PE=1 SV=3
MAKADKKPAEKKPAEKTPAAEPAAAAEKKPKAGKKLPKEPAGAGDKKKKRSKKNVETYKI
YIFKVLKQVHPDIGISSKAMGIMNSFINDIFEKLAGESSKLARYNKKPTITSREIQTAVR
LVLPGELAKHAVSEGTKAVTKFTSS

[출력] output_sequence/

example_seq1.txt (내용: MAPKAEKKPAEKKPASEKPV ...)

example_seq2.txt (내용: MAKADKKPAE ...)
