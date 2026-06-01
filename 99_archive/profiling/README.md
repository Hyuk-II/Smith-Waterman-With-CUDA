# 하드웨어 프로파일링 데이터

NVIDIA Nsight Compute 측정 데이터 및 분석 이미지를 보관합니다.

## 파일 목록

| 파일 | 설명 |
|---|---|
| `image.png` | `uint8_t` vs `int` DRAM Read Bytes 비교 스크린샷 (1D Grid 1×1 커널 기준) |
| `report_uint8.ncu-rep` | Nsight Compute 프로파일 — `uint8_t` 서열 인코딩 버전 |
| `report_int.ncu-rep` | Nsight Compute 프로파일 — `int` 서열 인코딩 대조군 버전 |

## 측정 요약

동일 서열 데이터에 대해 `int` 버전이 `uint8_t` 버전보다 DRAM 읽기량 약 **4.6배** 많음.  
→ `1_sw_implement/README.md` 의 [검증] 섹션 참조.
