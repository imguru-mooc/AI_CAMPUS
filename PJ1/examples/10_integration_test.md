# 통합·연속 동작 테스트 기록 (예시)

> 프로젝트 1 · 현장 안전 모니터링 온디바이스 AI  
> Day 10 · 12.08(화) · 제출: Day 10 8교시 (M5)

**이 문서는 작성 예시입니다. 조 이름, 수치, 파일명은 형식을 보여 주기 위한 가상 값입니다.**

| 조 | 작성자 | 작성일 |
| --- | --- | --- |
| 0조 (예시) | 조원 F | 12.08(화) |

## systemd 서비스 파일

```ini
[Unit]
Description=SafeCam safety monitor
After=multi-user.target

[Service]
User=pi
WorkingDirectory=/home/pi/safecam
ExecStart=/home/pi/safecam/.venv/bin/python main.py --config config.yaml
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

## 통합 점검

- [x] 모든 모듈이 `main.py` 하나로 실행됨
- [x] 임계값·핀 번호·구역 좌표가 설정 파일에 있음
- [x] `systemctl enable`로 등록함
- [x] 전원을 껐다 켜면 자동으로 실행됨
- [x] 프로그램을 강제 종료하면 자동으로 다시 시작됨
- [x] 종료 시 GPIO 출력이 모두 꺼짐

## 30분 연속 동작

| 경과(분) | FPS | CPU(%) | 메모리(MB) | 온도(℃) | 스로틀링 |
| --- | --- | --- | --- | --- | --- |
| 0 | 6.5 | 310 | 412 | 49 | 0x0 |
| 10 | 6.4 | 315 | 415 | 64 | 0x0 |
| 20 | 6.3 | 318 | 416 | 67 | 0x0 |
| 30 | 6.3 | 316 | 416 | 68 | 0x0 |

_메모리가 계속 늘어나면 누수를 의심합니다. 스로틀링은 `vcgencmd get_throttled`로 확인합니다._

## 장애 대응

| 상황 | 기대 동작 | 결과 |
| --- | --- | --- |
| 실행 중 카메라 케이블 분리 | 오류 로그를 남기고 5초 후 재시작 | 통과 |
| 스냅샷 500장 초과 | 오래된 파일부터 삭제 | 통과 |
| 설정 파일 누락 | 안내 메시지 출력 후 종료 | 통과 |

## 결론

| 항목 | 내용 |
| --- | --- |
| M5 달성 여부 | 달성 |
| 남은 문제 | 20분 이후 온도가 67℃ 이상으로 유지됨. 시연 때 소형 팬 사용 예정 |
