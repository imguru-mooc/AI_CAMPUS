#!/bin/bash
# collect_loop.sh — S7 백그라운드(&·nohup) 실습 + S8 systemd 서비스용 (상시형)
# S7에서 추가: while true + sleep 5. 1회형 collect.sh와 이름을 구분
PATH=/usr/local/bin:/usr/bin:/bin
LOG_DIR=/home/user/logs
[ -d "$LOG_DIR" ] || mkdir -p "$LOG_DIR"
while true; do
  {
    cat /sys/class/thermal/thermal_zone0/temp \
      | awk -v t="$(date '+%F %T')" '{printf "%s temp=%.1fC\n", t, $1/1000}'
  } >> "$LOG_DIR/sensor.log" 2>&1
  sleep 5
done
