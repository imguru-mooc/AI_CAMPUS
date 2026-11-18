# 환경 구축 체크리스트 (양식)

> 프로젝트 1 · 현장 안전 모니터링 온디바이스 AI  
> Day 1 · 11.25(수) · 제출: Day 1 8교시

조의 모든 Pi가 같은 환경인지 확인합니다. Pi마다 한 장씩 작성합니다.

| 조 | 작성자 | 작성일 |
| --- | --- | --- |
|   |   |   |

## 장비 정보

| 항목 | 내용 |
| --- | --- |
| Pi 식별 | _호스트명 또는 라벨 번호_ |
| OS 버전 | _`cat /etc/os-release`_ |
| Python 버전 | _`python3 --version`_ |
| onnxruntime 버전 | _아래 명령으로 확인_ |
| 카메라 | _모델명_ |

## 설치 확인

- [ ] OS 업데이트 완료
- [ ] Python 가상환경 생성 (`python3 -m venv --system-site-packages .venv`)
- [ ] picamera2 import 성공
- [ ] OpenCV import 성공
- [ ] onnxruntime import 성공
- [ ] numpy import 성공
- [ ] 카메라로 사진 1장 촬영 성공
- [ ] 노트북에서 SSH 접속 성공
- [ ] 조 GitHub 저장소 clone, 첫 커밋 push 성공
- [ ] CPU 온도 확인 (`vcgencmd measure_temp`)

## 버전 확인 출력

_작성 안내: 구버전 OS에서는 `rpicam-hello` 대신 `libcamera-hello`를 사용합니다._

```text
$ python3 -c "import onnxruntime as ort, cv2, numpy; print(ort.__version__, cv2.__version__, numpy.__version__)"
(여기에 출력 붙여 넣기)

$ rpicam-hello --list-cameras
(여기에 출력 붙여 넣기)
```

## 설치 중 문제

| 문제 | 해결 방법 |
| --- | --- |
|   |   |
|   |   |
