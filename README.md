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

본 프로젝트는 유지보수성과 확장성에 집중하기 위해 핵심 연산 로직과 공통 유틸리티 함수를 분리하여, 모듈화 아키텍처를 채택하고 있습니다.

```text
sw_gpu_accelerator/
├── 0_preprocessing/
│   ├── input_fasta/          # 원본 .fasta 데이터 보관
│   ├── output_sequence/      # 파싱 완료된 순수 서열(.txt) 자동 저장소
│   ├── preprocessor.cpp      # C++17 기반 멀티 파일 전처리 엔진
│   └── README.md             # 전처리 단계 실행 상세 가이드
│
├── 1_sw_implement/
│   ├── sequence_codec.h      # 서열 문자열 <-> 정수(uint8_t) 양방향 매핑 모듈
│   ├── common.h              # 공통 유틸리티 모음
│   ├── sw_cpu.cpp            # [Core] CPU 기반 스미스-워터맨 알고리즘 메인 로직
│   ├── sw_gpu.cu             # [Core] CUDA 기반 GPU 가속 스미스-워터맨 메인 로직
│   └── README.md             # 알고리즘 빌드 및 CLI 인수 실행 가이드
│
└── README.md                 # 프로젝트 최상위 개요 (본 문서)
```

## 🛠️ 4. Quick Start (OS별 빌드 가이드)

### Step 1. Data Preprocessing

`0_preprocessing/input_fasta` 디렉토리에 벤치마크할 서열들의 `.fasta` 파일을 넣고 파이프라인을 가동합니다.

**🍏 Mac / 🐧 Linux 환경**
```bash
cd 0_preprocessing
g++ -std=c++17 preprocessor.cpp -o preprocessor
./preprocessor
```

**🪟 Windows 환경 (PowerShell 권장)**
Windows 환경의 기본 인코딩(CP949)과 C++ 소스코드(UTF-8) 충돌을 방지하기 위해 `-fexec-charset=cp949` 플래그를 추가합니다.
```powershell
cd 0_preprocessing
g++ -std=c++17 -fexec-charset=cp949 preprocessor.cpp -o preprocessor.exe
.\preprocessor.exe
```

_실행 완료 시 `output_sequence` 폴더에 `[파일명]_seq1.txt`, `[파일명]_seq2.txt` 등의 파일이 자동 생성됩니다._

---

### Step 2. Execution

`1_sw_implement` 디렉토리에서 전처리된 파일의 **이름만** 인수로 전달하여 실행합니다.

#### 🖥️ [CPU Baseline 실행]

**🍏 Mac / 🐧 Linux**
```bash
cd 1_sw_implement
g++ -std=c++17 sw_cpu.cpp -o sw_cpu
./sw_cpu example_seq1.txt example_seq2.txt
```

**🪟 Windows (PowerShell)**
```powershell
cd 1_sw_implement
g++ -std=c++17 -fexec-charset=cp949 sw_cpu.cpp -o sw_cpu.exe
.\sw_cpu.exe example_seq1.txt example_seq2.txt
```

#### 🚀 [GPU 가속 커널 실행]

> ⚠️ **Windows 환경 빌드 시 주의사항 (필독)**
> 1. 반드시 **"x64 Native Tools Command Prompt"** 터미널을 사용하여 64비트 컴파일 환경을 구성해야 합니다. (일반 cmd나 x86 터미널 사용 시 `ACCESS_VIOLATION` 에러 발생)
> 2. 경로에 한글이나 공백이 포함되어 있으면 CUDA 컴파일러 프론트엔드(`cudafe++`)가 비정상 종료될 수 있습니다. 프로젝트 폴더는 순수 영문 경로(예: `C:\CUDA_Projects\...`)에 위치해야 합니다.
> 3. 소스코드 내부의 주석을 파싱하는 과정에서 인코딩 충돌이 발생하지 않도록, `1_sw_implement` 내의 모든 헤더(`.h`)와 코드(`.cu`) 파일은 **UTF-8 with BOM** 형식으로 저장하거나 영어로 작성할 것을 권장합니다.
> 4. `nvcc`에 `-Xcompiler "/utf-8"` 옵션을 추가하면 사용자 코드는 정상 파싱되지만, NVIDIA 내부 헤더 파싱 중 충돌을 일으킬 수 있으므로 **기본 명령어만 사용하는 것을 권장**합니다.

**🐧 Linux 환경** (아키텍처에 맞게 `-arch` 수정)
```bash
nvcc -std=c++17 -arch=sm_89 -O3 sw_gpu_tiled.cu -o sw_gpu_tiled
./sw_gpu_tiled example_seq1.txt example_seq2.txt
```

**🪟 Windows 환경 ("x64 Native Tools Command Prompt" 사용)**
```cmd
nvcc -std=c++17 -arch=sm_89 -O3 sw_gpu_tiled.cu -o sw_gpu_tiled.exe
.\sw_gpu_tiled.exe example_seq1.txt example_seq2.txt
```

## 📊 5. Evaluation Metrics

본 프로젝트의 성능 타당성은 다음 3가지 지표를 통해 분석 및 증명됩니다.

1. **GCUPS (Giga Cell Updates Per Second):** 초당 셀 계산량 (CPU 대비 전체 알고리즘의 실질적 가속비)
2. **Memory Bandwidth:** 전역 메모리 vs 공유 메모리 도입에 따른 대역폭 포화도 (Nsight Compute 활용)
3. **Hardware Occupancy:** 파도타기 비효율 구간(유휴 스레드)을 상쇄하기 위한 SM 스케줄링 점유율 분석
