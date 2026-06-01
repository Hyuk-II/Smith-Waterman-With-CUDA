# Linux x64 사전 빌드본

빌드 환경: **Linux x86-64 + CUDA Toolkit (`nvcc`, `-arch` 미지정 — PTX JIT 호환)**
빌드 머신 GPU: **NVIDIA GeForce RTX 3080 Ti** (Ampere, sm_86)

> ℹ️ `-arch` 플래그를 지정하지 않고 컴파일된 바이너리입니다. CUDA 기본 아키텍처 + PTX가 포함되어 있어 대부분의 CUDA GPU에서 JIT 컴파일로 실행 가능합니다.

## 파일별 런타임 요구사항

| 파일 | 종류 | 런타임 요구사항 |
|---|---|---|
| `preprocessor` | CPU only | Linux x86-64 |
| `sw_cpu` | CPU only | Linux x86-64 |
| `sw_gpu` | CUDA GPU | Linux x86-64 + NVIDIA Driver + CUDA Runtime, **Compute Capability 8.9** (RTX 40 시리즈 / Ada Lovelace) |
| `sw_gpu_tiled` | CUDA GPU | Linux x86-64 + NVIDIA Driver + CUDA Runtime, **Compute Capability 8.9** (RTX 40 시리즈 / Ada Lovelace) |

> ℹ️ PTX 포함 빌드이므로 sm_89 전용이 아닙니다. 다양한 CUDA GPU에서 JIT 컴파일로 실행 가능하나, CUDA 버전 호환성에 따라 실행이 불가할 수 있습니다. 문제 발생 시 루트 `README.md`의 Quick Start를 참고하여 재컴파일하세요.

## 실행 방법

바이너리를 `1_sw_implement/` 디렉토리로 복사한 뒤 실행하세요.

```bash
cp 99_archive/prebuilt_linux_x64_sm89/sw_gpu_tiled 1_sw_implement/
cd 1_sw_implement
./sw_gpu_tiled example_seq1.txt example_seq2.txt
```

데이터 파일(`example_seq1.txt` 등)은 `0_preprocessing/output_sequence/` 에 위치해야 합니다. `1_sw_implement/`를 CWD로 실행할 때 바이너리가 `../0_preprocessing/output_sequence/` 를 자동으로 참조합니다.
