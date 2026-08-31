# Day 3 스크립트 진화표

| 세션 | 파일 | 이 세션에서 추가된 것 | 실행 방식 |
|---|---|---|---|
| S4 | collect_v1.sh | `#!/bin/bash` + `echo start` | `chmod +x` 후 `./collect.sh` |
| S5 | collect_v2.sh | `LOG_DIR` 변수 · `[ -d ] \|\| mkdir -p` · `for` | `./` vs `source` 비교 |
| S6 | collect_v3.sh | 파이프 본체 · `{ } >> log 2>&1` | 1회 실행 → `tail -5 sensor.log` |
| S7 | collect_v4.sh | `PATH=` 명시 · 절대경로 | crontab `* * * * *` |
| S7·S8 | collect_loop.sh | `while true … sleep 5` | `&` / `nohup` / systemd |
| S8 | collect.service | unit 파일 | `systemctl enable --now collect` |

수업 중에는 파일 하나(`collect.sh`)를 계속 편집한다. 이 폴더의 v1~v4는 "각 세션이 끝났을 때의 정답 스냅샷"이며,
루프형만 `collect_loop.sh`로 이름을 구분한다(cron은 1회형, systemd는 루프형).

교체 지점: `/home/user` → 훈련생 계정명, `/sys/class/thermal/thermal_zone0/temp` → 확정된 데이터 소스.
