#!/bin/bash
# collect.sh — S5 끝의 모습
# S5에서 추가: LOG_DIR 변수, [ -d ] || mkdir 조건, for 반복
LOG_DIR=~/logs                              # ./collect.sh 로 실행하면 이 변수는 자식 bash 안에만 존재
echo "$LOG_DIR"
[ -d "$LOG_DIR" ] || mkdir -p "$LOG_DIR"    # [ 의 $?가 0이 아닐 때만 mkdir 실행
for i in 1 2 3; do
  date >> "$LOG_DIR/test.log"
done
