#!/bin/bash
# collect.sh — S7 cron 등록용 최종 (1회 실행형)
# S7에서 추가: PATH 명시 + 절대경로 (cron은 터미널 환경을 모른다)
# crontab -e :  * * * * * /home/user/collect.sh
PATH=/usr/local/bin:/usr/bin:/bin
LOG_DIR=/home/user/logs        # ~ 대신 절대경로, 계정명으로 교체
[ -d "$LOG_DIR" ] || mkdir -p "$LOG_DIR"
{
  cat /sys/class/thermal/thermal_zone0/temp \
    | awk -v t="$(date '+%F %T')" '{printf "%s temp=%.1fC\n", t, $1/1000}'
} >> "$LOG_DIR/sensor.log" 2>&1
