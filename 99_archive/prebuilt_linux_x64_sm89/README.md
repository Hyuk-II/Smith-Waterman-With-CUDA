# Linux x64 사전 빌드본

빌드 환경: **Linux x86-64 + CUDA Toolkit (`-arch=sm_89`, `-O3`)**

## 파일별 런타임 요구사항

| 파일 | 종류 | 런타임 요구사항 |
|---|---|---|
| `preprocessor` | CPU only | Linux x86-64 |
| `sw_cpu` | CPU only | Linux x86-64 |
| `sw_gpu` | CUDA GPU | Linux x86-64 + NVIDIA Driver + CUDA Runtime, **Compute Capability 8.9** (RTX 40 시리즈 / Ada Lovelace) |
| `sw_gpu_tiled` | CUDA GPU | Linux x86-64 + NVIDIA Driver + CUDA Runtime, **Compute Capability 8.9** (RTX 40 시리즈 / Ada Lovelace) |

> ⚠️ `sw_gpu`, `sw_gpu_tiled`는 sm_89 전용으로 컴파일되어 있습니다. RTX 30/20 등 다른 아키텍처에서는 `no kernel image available for execution on the device` 오류가 발생할 수 있으므로, 루트 `README.md`의 Quick Start를 참고하여 해당 GPU의 `-arch` 값으로 재컴파일하세요.
