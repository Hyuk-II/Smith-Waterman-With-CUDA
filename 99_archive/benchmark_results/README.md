# 벤치마크 실행 결과

환경별 실행 결과 텍스트 파일과 요약 캡처를 함께 보관합니다.

## 저장 방법

전체 터미널 출력을 텍스트로 저장한 뒤, 벤치마크 요약 박스(점수/시간/GCUPS)만 캡처합니다.

```bash
# 텍스트 출력 저장 예시
./sw_cpu TITIN_MOUSE.txt TITIN_HUMAN.txt > sw_cpu_linux_titin.txt 2>&1
```

## 파일 명명 규칙

```
{구현}_{환경}_{서열}.txt   ← 전체 터미널 출력
{구현}_{환경}_{서열}.png   ← 벤치마크 요약 박스 캡처
```

예시:
- `sw_cpu_linux_titin.txt` / `sw_cpu_linux_titin.png`
- `sw_gpu_tiled_linux_titin.txt` / `sw_gpu_tiled_linux_titin.png`
