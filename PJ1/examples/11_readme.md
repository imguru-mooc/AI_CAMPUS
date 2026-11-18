# 저장소 README 양식 (예시)

> 프로젝트 1 · 현장 안전 모니터링 온디바이스 AI  
> Day 11 · 12.09(수) · 제출: Day 11 8교시

**이 문서는 작성 예시입니다. 조 이름, 수치, 파일명은 형식을 보여 주기 위한 가상 값입니다.**

| 조 | 작성자 | 작성일 |
| --- | --- | --- |
| 0조 (예시) | 조원 F | 12.09(수) |

## 프로젝트 소개

SafeCam은 Raspberry Pi 4에서 동작하는 현장 안전 모니터링 장치입니다. 안전모 미착용과 위험구역 진입을 감지해 LED, 부저, 경광등으로 알립니다.

## 하드웨어

| 부품 | 수량 | 비고 |
| --- | --- | --- |
| Raspberry Pi 4 | 1 |   |
| Pi 카메라 | 1 | CSI 연결 |
| LED (녹·황·적) | 3 | 330Ω 저항 |
| 능동 부저 | 1 |   |
| 릴레이 모듈, 경광등 | 1 |   |
| LCD1602 (I2C) | 1 |   |

## 설치

```bash
git clone https://github.com/(조 계정)/safecam.git
cd safecam
python3 -m venv --system-site-packages .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 실행

```bash
python main.py --config config.yaml

# 부팅 시 자동 실행
sudo cp deploy/safecam.service /etc/systemd/system/
sudo systemctl enable --now safecam
```

## 폴더 구조

```text
safecam/
├── main.py
├── config.yaml
├── src/        capture, infer, rules, alert, logger, viewer
├── models/     best_int8.onnx
├── config/     zone.json
├── deploy/     safecam.service
└── docs/       산출물 문서, 회로도
```

## 성능

| 지표 | 결과 |
| --- | --- |
| mAP50 | 0.81 |
| 처리 속도 | 6.3 FPS |
| 경보 지연 | 1.0초 |

## 팀

| 이름 | 역할 |
| --- | --- |
| 조원 A | 팀장 |
| 조원 B | 데이터 |
