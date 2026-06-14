# 하드웨어 프로파일링 데이터

NVIDIA Nsight Compute 측정 데이터 및 분석 이미지를 보관합니다.

## 파일 목록

| 파일 | 설명 |
|---|---|
| `int_vs_uint8.png` | `uint8_t` vs `int` DRAM Read Bytes 비교 스크린샷 (1D Grid 1×1 커널 기준) |
| `report_uint8.ncu-rep` | Nsight Compute 프로파일 — `uint8_t` 서열 인코딩 버전 |
| `report_int.ncu-rep` | Nsight Compute 프로파일 — `int` 서열 인코딩 대조군 버전 |
| `tiled_vs_global/` | 전역(`sw_gpu`) vs 공유 메모리 타일(`sw_gpu_tiled`) 커널 Baseline 비교 캡처 (지표 2·3) |

### `tiled_vs_global/` — 전역 메모리 vs 공유 메모리 타일링 비교

Nsight Compute Baseline 기능으로 두 커널의 중앙 반대각선(대표) 커널을 비교한 스크린샷. `Baseline = sw_gpu(전역)`, `Current = sw_gpu_tiled(공유)`.

| 파일 | 지표 | 설명 |
|---|---|---|
| `gpu_speed_of_light.png` | 2 | DRAM/L2/Memory/Compute 처리량 요약 (병목 이동) |
| `memory_workload_analysis.png` | 2 | DRAM Throughput ↓, L2 Hit Rate ↑, Shared 사용 |
| `occupancy.png` | 3 | Theoretical 37.5 % / Achieved 2.82 %, Shared Mem 제한 |
| `launch_statistics.png` | 3 | grid 크기·Waves per SM (GPU 미충전 근거) |

→ 분석 요약은 최상위 `README.md`의 **5. Evaluation Metrics** 섹션 참조.

## 측정 요약

동일 서열 데이터에 대해 `int` 버전이 `uint8_t` 버전보다 DRAM 읽기량 약 **4.6배** 많음.  
→ `1_sw_implement/README.md` 의 [검증] 섹션 참조.
