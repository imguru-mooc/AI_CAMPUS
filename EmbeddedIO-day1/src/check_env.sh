#!/bin/bash
# check_env.sh — Embedded I/O 제어 Day 1 실습 환경 점검
# 대상: Raspberry Pi 4 + Raspberry Pi OS Lite 64-bit (Trixie)
# 사용법: bash check_env.sh

PASS=0; FAILN=0
ok()   { printf "  [ OK ] %s\n" "$*"; PASS=$((PASS + 1)); }
warn() { printf "  [WARN] %s\n" "$*"; }
fail() { printf "  [FAIL] %s\n" "$*"; FAILN=$((FAILN + 1)); }

echo "== 1. 보드와 OS =="
MODEL=$( (tr -d '\0' < /proc/device-tree/model) 2>/dev/null )
if [[ "$MODEL" == "Raspberry Pi 4"* ]]; then ok "보드   : $MODEL"
else fail "보드   : ${MODEL:-알 수 없음} (Raspberry Pi 4 가 아님)"; fi
. /etc/os-release
if [[ "$VERSION_CODENAME" == "trixie" ]]; then ok "OS     : $PRETTY_NAME"
else fail "OS     : $PRETTY_NAME (수업 기준은 trixie — libgpiod v2)"; fi
if [[ "$(uname -m)" == "aarch64" ]]; then ok "커널   : $(uname -r) (aarch64)"
else warn "커널   : $(uname -r) ($(uname -m)) — 64-bit 이미지 권장"; fi

echo "== 2. 전원과 온도 =="
if command -v vcgencmd >/dev/null 2>&1; then
    T=$(vcgencmd get_throttled | cut -d= -f2)
    if [[ "$T" == "0x0" ]]; then ok "전원   : 저전압 기록 없음 (throttled=0x0)"
    else warn "전원   : throttled=$T — PC USB 전원이었다면 어댑터로 바꾼 뒤 재부팅해 다시 확인 (5V 3A)"; fi
fi
TZ0=/sys/class/thermal/thermal_zone0/temp
if [ -r "$TZ0" ]; then
    TEMP=$(( $(cat "$TZ0") / 1000 ))
    if [ "$TEMP" -lt 70 ]; then ok "온도   : ${TEMP} °C"
    else warn "온도   : ${TEMP} °C — 쿨러 확인"; fi
else warn "온도   : 읽을 수 없음"; fi

echo "== 3. 네트워크와 호스트 이름 =="
HN=$(hostname)
if [[ "$HN" =~ ^rpi-[0-9]{2}$ ]]; then ok "호스트 : $HN (좌석 번호)"
else warn "호스트 : $HN — 좌석 번호로: sudo raspi-config nonint do_hostname rpi-NN"; fi
if command -v nmcli >/dev/null 2>&1 && nmcli -t -f DEVICE,STATE dev 2>/dev/null | grep -q '^wlan0:connected'; then
    ok "Wi-Fi  : wlan0 연결됨 ($(ip -4 -o addr show wlan0 | awk '{print $4}'))"
else fail "Wi-Fi  : wlan0 미연결 — sudo nmcli dev wifi connect \"SSID\" password \"비밀번호\""; fi
if ip link show usb0 >/dev/null 2>&1; then
    warn "USB    : gadget mode 켜짐 — Wi-Fi 접속 확인 후 sudo rpi-usb-gadget off"
else ok "USB    : gadget mode 꺼짐 (USB-C = 전원 입력)"; fi

echo "== 4. 인터페이스 장치 파일 =="
check_node() {
    if [ -e "$1" ]; then ok "$1  ($2)"; else fail "$1 없음 — $3"; fi
}
check_node /dev/gpiochip0          "GPIO" "커널 문제, 강사에게 알리기"
check_node /dev/i2c-1              "I2C"  "sudo raspi-config nonint do_i2c 0"
check_node /dev/spidev0.0          "SPI"  "sudo raspi-config nonint do_spi 0"
check_node /dev/serial0            "UART" "sudo raspi-config nonint do_serial_hw 0"
check_node /sys/class/pwm/pwmchip0 "PWM"  "config.txt 에 dtoverlay=pwm-2chan"
if [ -e /dev/serial0 ]; then
    S=$(readlink /dev/serial0)
    if [[ "$S" == "ttyAMA0" ]]; then ok "serial0 → ttyAMA0 (PL011)"
    else warn "serial0 → $S — config.txt 에 dtoverlay=disable-bt"; fi
fi
if grep -q "console=serial0" /boot/firmware/cmdline.txt 2>/dev/null; then
    warn "시리얼 콘솔 켜짐 — sudo raspi-config nonint do_serial_cons 1"
else ok "시리얼 콘솔 꺼짐"; fi

echo "== 5. 사용자 권한 =="
for g in gpio i2c spi dialout; do
    if id -nG | grep -qw "$g"; then ok "그룹 $g"
    else fail "그룹 $g 아님 — sudo usermod -aG $g \$USER 후 다시 로그인"; fi
done

echo "== 6. 개발 도구 =="
for t in gcc make gpiodetect gpioset i2cdetect; do
    if command -v "$t" >/dev/null 2>&1; then ok "$t"; else fail "$t 없음"; fi
done
V=$(pkg-config --modversion libgpiod 2>/dev/null)
if [[ "$V" == 2.* ]]; then ok "libgpiod-dev $V (v2 API)"
elif [ -n "$V" ]; then fail "libgpiod-dev $V — v1 API 라 수업 예제가 컴파일되지 않음"
else fail "libgpiod-dev 없음"; fi

echo
echo "결과: OK ${PASS}개, FAIL ${FAILN}개"
if [ "$FAILN" -eq 0 ]; then echo "Day 2 준비 완료"
else echo "설치: sudo apt install -y build-essential gpiod libgpiod-dev i2c-tools"; fi
