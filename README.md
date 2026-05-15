# 🧬 CUDA-Accelerated Smith-Waterman Alignment

> **Smith-Waterman 알고리즘 기반 단백질/유전체 서열 정렬 GPU 가속기**

## 💡 1. Background & Objective

기존의 동적 계획법 기반 스미스-워터맨(Smith-Waterman) 알고리즘은 데이터 의존성으로 인해 병목이 존재합니다.

본 프로젝트는 알고리즘의 **반대각선 독립성**을 기반으로, 이를 독립적으로 연산가능한 타일을 GPU의 코어에 매핑하여 연산 스텝을 $O(N^2)$에서 $O(N)$으로 개선합니다.

느린 전역 메모리 의존도를 낮추기 위해 **Shared Memory 기반의 Tiling** 기법을 적용하여 높은 연산 처리량을 달성하는 것을 목표로 합니다.

## 🚀 2. 핵심 최적화 전략 (Core Optimizations)

- **Wave-front Parallelization:** 의존성이 없는 $i+j=k$ 선상의 셀들을 동일한 Phase로 묶어 GPU 스레드 블록에 분산 처리.
- **Shared Memory Tiling:** 데이터 I/O 지연을 은닉하기 위해, 16x16 크기의 타일 단위로 데이터를 글로벌 메모리에서 공유 메모리로 일괄 적재 후 연산 수행.
- **Decoupled Preprocessing:** 대용량 FASTA 파일(테스트 데이터 출처: [UniProt](https://www.uniprot.org/), [NCBI](https://www.ncbi.nlm.nih.gov/))에서 헤더와 기호를 제거하고 순수 염기/아미노산 서열만 추출하는 C++17 파서 구축.
- **ASCII 직접 참조 인코딩 ($O(1)$ Look-up):** GPU Branch를 막기 위해, 알파벳을 즉시 1바이트 숫자(`uint8_t`)로 변환하는 하드웨어 친화적 인코더 설계.

## 📂 3. 디렉토리 구조 (Directory Structure)

프로젝트는 데이터 정제(Preprocessing)와 실제 연산(Implementation) 파이프라인이 분리되어 있습니다.

```text
sw_gpu_accelerator/
├── 0_preprocessing/
│   ├── input_fasta/          # 사용자가 다운로드한 원본 .fasta 파일 위치
│   ├── output_sequence/      # 추출된 순수 염기/아미노산 서열(.txt) 저장소
│   ├── preprocessor.cpp      # C++17 기반 대량 FASTA 자동 파싱 및 추출기
│   └── README.md             # 전처리 파이프라인 사용 가이드
│
├── 1_sw_implement/
│   ├── sequence_encoder.h    # ASCII -> uint8_t 초고속 변환 클래스
│   ├── sw_cpu.cpp            # CPU 순차 처리 Baseline (결과 검증용)
│   ├── sw_gpu.cu             # CUDA 기반 핵심 병렬 가속 커널
│   └── README.md             # 연산기 빌드 및 실행 가이드
│
└── README.md                 # 프로젝트 최상위 개요 문서 (Current File)

```

## 🛠️ 4. Quick Start

### Step 1. Data Preprocessing

`0_preprocessing/input_fasta` 디렉토리에 벤치마크할 서열들(예: 애기장대 히스톤, 코로나 바이러스 등)의 `.fasta` 파일을 넣고 파이프라인을 가동합니다.

```bash
cd 0_preprocessing
g++ -std=c++17 preprocessor.cpp -o preprocessor
./preprocessor

```

_실행 완료 시 `output_sequence` 폴더에 `[파일명]_seq1.txt`, `[파일명]_seq2.txt` 등의 파일이 자동 생성됩니다._

### Step 2. Execution via CLI

전처리된 파일의 **상대 경로를 명령줄 인수(Command Line Arguments)로 직접 넘겨주어** 연산을 수행합니다.

**[CPU Baseline 실행]**

```bash
cd ../1_sw_implement
g++ -std=c++17 sw_cpu.cpp -o sw_cpu
./sw_cpu ../0_preprocessing/output_sequence/H2B10_seq1.txt ../0_preprocessing/output_sequence/H2B11_seq2.txt

```

**[GPU 가속 커널 실행]**

```bash
nvcc -std=c++17 sw_gpu.cu -o sw_gpu
./sw_gpu ../0_preprocessing/output_sequence/H2B10_seq1.txt ../0_preprocessing/output_sequence/H2B11_seq2.txt

```

## 📊 5. Evaluation Metrics

본 프로젝트의 성능 타당성은 다음 3가지 지표를 통해 분석 및 증명됩니다.

1. **GCUPS (Giga Cell Updates Per Second):** 초당 셀 계산량 (CPU 대비 전체 알고리즘의 실질적 가속비)
2. **Memory Bandwidth:** 전역 메모리 vs 공유 메모리 도입에 따른 대역폭 포화도 (Nsight Compute 활용)
3. **Hardware Occupancy:** 파도타기 비효율 구간(유휴 스레드)을 상쇄하기 위한 SM 스케줄링 점유율 분석
