# macOS Apple Silicon 사전 빌드본

빌드 환경: **macOS arm64 + Apple Clang (`g++ -std=c++17`)**

## 파일별 런타임 요구사항

| 파일 | 종류 | 런타임 요구사항 |
|---|---|---|
| `sw_cpu` | CPU only | macOS arm64 (Apple Silicon: M1/M2/M3/M4 등) |

## macOS GPU 가속 미제공 사유

NVIDIA는 macOS 10.13 (High Sierra) 이후 CUDA Toolkit을 공식 지원하지 않으며, Apple Silicon은 NVIDIA GPU를 탑재하지 않습니다. 따라서 본 디렉토리에는 GPU 커널(`sw_gpu`, `sw_gpu_tiled`) 바이너리가 제공되지 않습니다. GPU 가속 실행이 필요하다면 Linux 또는 Windows 환경의 prebuilt를 사용하거나 직접 재컴파일하세요.
