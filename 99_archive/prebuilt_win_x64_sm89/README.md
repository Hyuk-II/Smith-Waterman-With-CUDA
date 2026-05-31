# Windows x64 사전 빌드본

빌드 환경: **Windows x86-64 + CUDA Toolkit (`-arch=sm_89`, `-O3`), "x64 Native Tools Command Prompt"**

## 파일별 런타임 요구사항

| 파일 | 종류 | 런타임 요구사항 |
|---|---|---|
| `preprocessor.exe` | CPU only | Windows x86-64 |
| `sw_cpu.exe` | CPU only | Windows x86-64 |
| `sw_gpu_tiled.exe` | CUDA GPU | Windows x86-64 + NVIDIA Driver + CUDA Runtime, **Compute Capability 8.9** (RTX 40 시리즈 / Ada Lovelace) |
| `sw_gpu_tiled_int.exe` | CUDA GPU (비교 실험용) | 동일. `uint8_t` 대신 `int`로 빌드된 대조군 — Nsight Compute DRAM 읽기량 비교에 사용 |

## 부속 파일 (자동 생성된 링커 산물)

| 파일 | 설명 |
|---|---|
| `*.lib`, `*.exp` | nvcc/MSVC 링커가 자동 생성. 실행에는 불필요. 빌드 산물 보존 차원으로만 동봉 |

> ⚠️ GPU 바이너리는 sm_89 전용으로 컴파일되어 있습니다. RTX 30/20 등 다른 아키텍처에서는 `no kernel image available for execution on the device` 오류가 발생할 수 있으므로, 루트 `README.md`의 Quick Start를 참고하여 해당 GPU의 `-arch` 값으로 재컴파일하세요.

## 실행 방법 (경로 처리 주의)

본 빌드본은 **이전 버전 소스** 기준이라 절대 경로/임의 상대 경로를 직접 받지 못합니다. 데이터 파일은 항상 `..\0_preprocessing\output_sequence\` 위치에서 파일명만으로 조회되므로, **반드시 `1_sw_implement\`를 CWD로 설정하고 실행하거나 데이터 파일을 적절히 배치**해야 합니다.

```cmd
:: 권장: 소스 트리의 1_sw_implement를 CWD로 설정 후 실행
cd 1_sw_implement
..\99_archive\prebuilt_win_x64_sm89\sw_gpu_tiled.exe example_seq1.txt example_seq2.txt
```

> ℹ️ 절대 경로 입력을 지원하려면 [common.h](../../1_sw_implement/common.h)의 `get_sequences()` 변경분을 반영하여 재컴파일하세요. macOS arm64용 `sw_cpu`는 이미 신코드로 갱신되어 있습니다.
