# 환경 구축 체크리스트 (예시)

> 프로젝트 1 · 현장 안전 모니터링 온디바이스 AI  
> Day 1 · 11.25(수) · 제출: Day 1 8교시

**이 문서는 작성 예시입니다. 조 이름, 수치, 파일명은 형식을 보여 주기 위한 가상 값입니다.**

| 조 | 작성자 | 작성일 |
| --- | --- | --- |
| 0조 (예시) | 조원 F | 11.25(수) |

## 장비 정보

| 항목 | 내용 |
| --- | --- |
| Pi 식별 | pi-team0-1 |
| OS 버전 | Raspberry Pi OS (64-bit) Bookworm |
| Python 버전 | 3.11.2 |
| onnxruntime 버전 | 1.17.1 |
| 카메라 | Pi Camera Module V2 (IMX219) |

## 설치 확인

- [x] OS 업데이트 완료
- [x] Python 가상환경 생성 (`python3 -m venv --system-site-packages .venv`)
- [x] picamera2 import 성공
- [x] OpenCV import 성공
- [x] onnxruntime import 성공
- [x] numpy import 성공
- [x] 카메라로 사진 1장 촬영 성공
- [x] 노트북에서 SSH 접속 성공
- [x] 조 GitHub 저장소 clone, 첫 커밋 push 성공
- [x] CPU 온도 확인 (`vcgencmd measure_temp`)

## 버전 확인 출력

```text
$ python3 -c "import onnxruntime as ort, cv2, numpy; print(ort.__version__, cv2.__version__, numpy.__version__)"
1.17.1 4.6.0 1.24.2

$ rpicam-hello --list-cameras
Available cameras
-----------------
0 : imx219 [3280x2464 10-bit RGGB]

$ vcgencmd measure_temp
temp=48.3'C
```

## 설치 중 문제

| 문제 | 해결 방법 |
| --- | --- |
| 가상환경에서 picamera2를 찾지 못함 | `--system-site-packages` 옵션으로 가상환경 재생성 |
