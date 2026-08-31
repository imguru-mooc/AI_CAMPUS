#!/bin/bash
# collect.sh — S6 끝의 모습 (1회 실행형)
# S6에서 추가: 파이프 본체, { } >> log 2>&1 로 stdout·stderr를 한 파일에
# 데이터 소스 [강사 확정]: 센서 미준비 시 SoC 온도로 대체, 확정되면 cat 줄만 교체
LOG_DIR=~/logs
[ -d "$LOG_DIR" ] || mkdir -p "$LOG_DIR"
{
  cat /sys/class/thermal/thermal_zone0/temp \
    | awk -v t="$(date '+%F %T')" '{printf "%s temp=%.1fC\n", t, $1/1000}'
} >> "$LOG_DIR/sensor.log" 2>&1
