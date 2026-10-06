#!/bin/bash
# memlab_run.sh — 실험마다 N회 반복해 CSV 로 저장
# 사용법: ./memlab_run.sh [반복횟수(기본 5)] [출력CSV(기본 memlab.csv)]
N=${1:-5}
OUT=${2:-memlab.csv}
echo "exp,size_mb,minflt,majflt,rss_kb,ms" > "$OUT"
for e in demand cow mmap; do
    for i in $(seq 1 "$N"); do ./memlab "$e" >> "$OUT"; done
done
if sudo -n true 2>/dev/null; then
    for i in $(seq 1 "$N"); do sudo ./memlab lock >> "$OUT"; done
else
    echo "(lock 실험은 sudo 가 필요해 건너뜀 — sudo -v 후 다시 실행)"
fi
echo "저장: $OUT"
column -s, -t < "$OUT"
