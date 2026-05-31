# macOS Apple Silicon 사전 빌드본

빌드 환경: **macOS arm64 + Apple Clang (`g++ -std=c++17`)**

## 파일별 런타임 요구사항

| 파일 | 종류 | 런타임 요구사항 |
|---|---|---|
| `sw_cpu` | CPU only | macOS arm64 (Apple Silicon: M1/M2/M3/M4 등) |

## 실행 방법

본 디렉토리에서 직접 실행 가능합니다. 데이터 파일은 **절대 경로 또는 현재 위치 기준 상대 경로**로 지정하세요.

```bash
# (a) 절대 경로
./sw_cpu /path/to/seq1.txt /path/to/seq2.txt

# (b) 프로젝트 루트 기준 상대 경로
./sw_cpu ../../0_preprocessing/output_sequence/example_seq1.txt \
         ../../0_preprocessing/output_sequence/example_seq2.txt
```

`example_seq1.txt`처럼 파일명만 전달하면 `../0_preprocessing/output_sequence/` 기본 위치로 fallback이 동작하나, 본 디렉토리에서는 해당 경로가 존재하지 않으므로 실패합니다. 이 단축 형태는 `cd 1_sw_implement` 후 실행할 때만 사용하세요.

## macOS GPU 가속 미제공 사유

NVIDIA는 macOS 10.13 (High Sierra) 이후 CUDA Toolkit을 공식 지원하지 않으며, Apple Silicon은 NVIDIA GPU를 탑재하지 않습니다. 따라서 본 디렉토리에는 GPU 커널(`sw_gpu`, `sw_gpu_tiled`) 바이너리가 제공되지 않습니다. GPU 가속 실행이 필요하다면 Linux 또는 Windows 환경의 prebuilt를 사용하거나 직접 재컴파일하세요.
