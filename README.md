# 🧬 CUDA-Accelerated Smith-Waterman Alignment

> **Smith-Waterman 알고리즘 기반 단백질/유전체 서열 정렬 GPU 가속기**

## 💡 1. Background & Objective

기존의 동적 계획법 기반 스미스-워터맨(Smith-Waterman) 알고리즘은 데이터 의존성으로 인해 병목이 존재합니다.

본 프로젝트는 알고리즘의 **반대각선 독립성**을 기반으로, 이를 독립적으로 연산가능한 타일을 GPU의 코어에 매핑하여 연산 스텝을 $O(N^2)$에서 $O(N)$으로 개선합니다.

느린 전역 메모리 의존도를 낮추기 위해 **Shared Memory 기반의 Tiling** 기법을 적용하여 높은 연산 처리량을 달성하는 것을 목표로 합니다.

## 🚀 2. 핵심 최적화 전략 (Core Optimizations)

- **Wave-front Parallelization:** 의존성이 없는 $i+j=k$ 선상의 셀들을 동일한 Phase로 묶어 GPU 스레드 블록에 분산 처리.
- **Shared Memory Tiling:** 데이터 I/O 지연을 은닉하기 위해, 32x32 크기의 타일 단위로 데이터를 글로벌 메모리에서 공유 메모리로 일괄 적재 후 연산 수행.
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
│   ├── sw_gpu_tiled.cu       # [Core] GPU 가속 메인 로직 공유 메모리 사용으로 개선
│   └── README.md             # 알고리즘 빌드 및 CLI 인수 실행 가이드
│
├── 99_archive/
│   ├── prebuilt_win_x64_sm89/    # Windows x64 사전 빌드본 (GPU 커널은 sm_89 전용, RTX 4080 Super)
│   ├── prebuilt_linux_x64/       # Linux x64 사전 빌드본 (nvcc 기본 arch, PTX JIT 호환, RTX 3080 Ti)
│   ├── prebuilt_macos_arm64/     # macOS Apple Silicon CPU 베이스라인 사전 빌드본
│   ├── benchmark_results/        # 벤치마크 터미널 출력(.txt) 및 요약 캡처(.png) 보관
│   ├── profiling/                # Nsight Compute 프로파일 데이터 및 uint8_t vs int 비교 이미지
│   └── 2021111971_이재혁_SW가속화.pdf  # 프로젝트 제안서
│
└── README.md                 # 프로젝트 최상위 개요 (본 문서)
```

> ℹ️ **사전 빌드 바이너리 안내 (`99_archive/prebuilt_*`)**
>
> 동일 환경에서 즉시 실행 가능한 컴파일 산물이 OS별 디렉토리에 보관되어 있습니다. **각 디렉토리 내부의 `README.md`** 에 파일별 런타임 요구사항이 표로 정리되어 있으니, 사용 전 해당 문서를 확인하세요.
>
> - **Windows (`prebuilt_win_x64_sm89`)**: RTX 4080 Super (Ada Lovelace) 빌드 머신, `-arch=sm_89` 지정 컴파일 — **sm_89 (RTX 40 시리즈) 전용**. 다른 아키텍처에서는 재컴파일 필요.
> - **Linux (`prebuilt_linux_x64_sm89`)**: RTX 3080 Ti (Ampere) 빌드 머신, `-arch` 플래그 미지정 — CUDA 기본 arch + PTX 포함으로 **대부분의 CUDA GPU에서 JIT 실행 가능**.
> - macOS는 CUDA를 공식 지원하지 않으므로 GPU 커널 바이너리는 제공되지 않습니다 (CPU 베이스라인만 제공).

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
>
> 1. 반드시 **"x64 Native Tools Command Prompt"** 터미널을 사용하여 64비트 컴파일 환경을 구성해야 합니다. (일반 cmd나 x86 터미널 사용 시 `ACCESS_VIOLATION` 에러 발생, CUDA Toolkit은 공식적으로 32비트 환경을 지원하지 않습니다.)
> 2. 경로에 한글이나 공백이 포함되어 있으면 CUDA 컴파일러 프론트엔드(`cudafe++`)가 비정상 종료될 수 있습니다. 프로젝트 폴더는 순수 영문 경로(예: `C:\CUDA_Projects\...`)에 위치해야 합니다.
> 3. 소스코드 내부의 주석을 파싱하는 과정에서 인코딩 충돌이 발생하지 않도록, `1_sw_implement` 내의 모든 헤더(`.h`)와 코드(`.cu`) 파일은 **UTF-8 with BOM** 형식으로 저장하거나 영어로 작성할 것을 권장합니다.
> 4. `nvcc`에 `-Xcompiler "/utf-8"` 옵션을 추가하면 사용자 코드는 정상 파싱되지만, NVIDIA 내부 헤더 파싱 중 충돌을 일으킬 수 있으므로 **기본 명령어만 사용하는 것을 권장**합니다.

**🐧 Linux 환경** (GPU 아키텍처에 맞게 `-arch` 수정 — 예: RTX 30 시리즈는 `sm_86`, RTX 40 시리즈는 `sm_89`)

```bash
nvcc -std=c++17 -arch=sm_86 -O3 sw_gpu_tiled.cu -o sw_gpu_tiled
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

### 📈 벤치마크 결과 (TITIN_MOUSE × TITIN_HUMAN — 35,213 × 34,350 AA — RTX 3080 Ti / Linux)

Titin은 알려진 단백질 중 최대 크기(~35,000 AA)로, 스코어 테이블 크기 약 4.5 GB의 실전 규모 벤치마크입니다.

| 구현 | 연산 시간 | GCUPS | CPU 대비 |
|---|---|---|---|
| `sw_cpu` (CPU Baseline) | 70,117 ms | 0.017 | 1× |
| `sw_gpu` (Wavefront GPU) | 20,386 ms | 0.059 | **3.4×** |
| `sw_gpu_tiled` (Shared Memory Tiling) | 15,310 ms | 0.079 | **4.6×** |

> ℹ️ **측정 방법:** `cudaFree(0)` warmup 호출로 CUDA context 초기화 패널티를 타이머 밖으로 배제 후 측정. GPU 시간은 H2D 복사 + 커널 실행 + D2H 복사 전체 파이프라인 포함. 세 구현 모두 최대 정렬 점수 **165,624**, 정렬 길이 **35,318 AA**로 일치하여 병렬화 정확도 검증 완료.
>
> **Tiled GPU vs Wavefront GPU (1.33×):** 이 규모에서 sw_gpu는 커널 런치 **69,562회**, sw_gpu_tiled는 **~2,172회** (32×32 타일 블록 단위). 스코어 테이블이 4.5 GB로 GPU L2 캐시(6 MB)를 압도하므로 모든 접근이 DRAM에서 이루어지며, Tiling의 coalesced 접근 패턴이 대역폭 효율을 높여 격차를 만들어냅니다.
