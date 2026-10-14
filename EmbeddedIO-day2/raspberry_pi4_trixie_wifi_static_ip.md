# Raspberry Pi 4B — Wi-Fi 자동 연결, nmap 활성 장비 검색 및 고정 IP 설정

**대상 OS:** Raspberry Pi OS Trixie 64-bit Lite

## 1. 네트워크 설정 정보

| 항목 | 설정값 |
|---|---|
| Wi-Fi SSID | `hyundai_02` |
| Wi-Fi 비밀번호 | `hyundai02!` |
| 네트워크 대역 | `192.168.20.0/24` |
| 고정 IP 후보 | `192.168.20.100` |
| 게이트웨이 | `192.168.20.1` (예시: 실제 값 확인) |
| Wi-Fi 인터페이스 | `wlan0` |
| DNS | `8.8.8.8`, `1.1.1.1` |

> 주의: IP, 서브넷, 게이트웨이는 현재 네트워크 정보와 일치해야 합니다. `192.168.20.100`은 다른 장비와 중복되지 않고 DHCP 예약/할당과 충돌하지 않는 경우에만 사용하세요.

## 2. Wi-Fi 활성화 및 연결

```bash
# Wi-Fi 활성화
sudo rfkill unblock wifi
sudo nmcli radio wifi on

# Wi-Fi 연결 (비밀번호를 대화식으로 입력)
sudo nmcli --ask device wifi connect 'hyundai_02' ifname wlan0
```

비밀번호 입력 시 `hyundai02!`를 입력합니다.

현재 IP, 라우팅 정보 및 연결 프로필 확인:

```bash
ip -4 addr show wlan0
ip route
nmcli connection show
```

## 3. Wi-Fi 연결 프로필 확인 및 생성 (`unknown connection` 해결)

`nmcli connection modify 'hyundai_02'` 명령에서 다음 오류가 발생한다면:

```text
Error: unknown connection 'hyundai_02'.
```

이는 **SSID가 보이더라도 `hyundai_02`라는 이름의 NetworkManager 연결 프로필이 존재하지 않는다는 뜻**입니다. SSID(무선 네트워크 이름)와 연결 프로필 이름은 서로 다를 수 있습니다.

### ① 현재 장치와 프로필 확인

```bash
nmcli device status
nmcli connection show
nmcli -g GENERAL.CONNECTION device show wlan0
nmcli -f IN-USE,SSID,SIGNAL,SECURITY device wifi list ifname wlan0
```

예를 들어 `nmcli connection show` 결과의 `NAME`이 `preconfigured`라면, 실제 연결에 사용되는 프로필 이름은 `hyundai_02`가 아닌 `preconfigured`일 수 있습니다. 기존 프로필이 정상적으로 Wi-Fi에 연결되어 있다면, **새 프로필을 중복 생성하지 말고 기존 프로필 이름으로** 이후 `connection modify` 명령을 실행하세요.

### ② 프로필이 없으면 먼저 일반 연결 시도

```bash
sudo nmcli radio wifi on
sudo nmcli device wifi rescan ifname wlan0
sudo nmcli --ask device wifi connect 'hyundai_02' ifname wlan0
```

비밀번호를 물으면 `hyundai02!`를 입력합니다. 성공하면 다음으로 생성된 프로필 이름을 확인합니다.

```bash
nmcli connection show
nmcli -g GENERAL.CONNECTION device show wlan0
```

### ③ 연결 명령이 실패했고 프로필도 없을 때 직접 생성

**아래 명령은 `hyundai_02`라는 이름의 연결 프로필이 없는 경우에만 사용합니다.** 이미 있다면 중복 생성하지 마세요.

```bash
sudo nmcli connection add \
    type wifi \
    con-name 'hyundai_02' \
    ifname wlan0 \
    ssid 'hyundai_02' \
    wifi-sec.key-mgmt wpa-psk \
    wifi-sec.psk 'hyundai02!' \
    ipv4.method auto \
    connection.autoconnect yes
```

> 위 명령은 WPA/WPA2-Personal(PSK) 방식의 일반 Wi-Fi를 가정합니다. WPA3 전용 또는 기업용 802.1X 네트워크라면 보안 설정이 달라집니다. 비밀번호를 명령어에 직접 입력하면 셸 기록에 남을 수 있으므로 공유 환경에서는 주의하세요.

먼저 **DHCP 방식으로 접속이 정상 동작하는지** 확인합니다.

```bash
sudo nmcli connection up 'hyundai_02'
ip -4 addr show wlan0
ip route
```

연결에 성공한 이후에만 아래 고정 IP 단계로 진행하세요. 현재 네트워크가 실제로 `192.168.20.0/24`이고 게이트웨이가 `192.168.20.1`인지 확인해야 합니다.

### ④ 연결 실패 시 오류 로그 확인

```bash
sudo journalctl -u NetworkManager -b --no-pager -n 50
```

| 오류 | 확인 사항 |
|---|---|
| `unknown connection` | 프로필 이름이 다른지 또는 프로필이 없는지 확인 |
| `Secrets were required` | 비밀번호나 인증 설정 확인 |
| `No network with SSID` | SSID 검색 가능 여부, 신호, 국가 설정 확인 |
| `Activation failed` | 상세 로그에서 인증·DHCP 등 실패 원인 확인 |

---

## 4. nmap으로 활성 장비 검색

### ① nmap 설치

```bash
sudo apt update
sudo apt install -y nmap
```

### ② 현재 Wi-Fi 대역의 활성 장비 검색

```bash
sudo nmap -sn 192.168.20.0/24
```

- `-sn`: 포트 스캔 없이 활성 호스트 탐색
- `192.168.20.0/24`: `192.168.20.*` 대역 탐색

예상 결과:

```text
Nmap scan report for 192.168.20.1
Host is up

Nmap scan report for 192.168.20.20
Host is up

Nmap scan report for 192.168.20.50
Host is up

Nmap scan report for 192.168.20.101
Host is up
```

### ③ 활성 장비 IP만 출력

```bash
sudo nmap -sn -n 192.168.20.0/24 -oG - \
    | awk '/Up$/{print $2}' \
    | sort -V
```

예상 출력:

```text
192.168.20.1
192.168.20.20
192.168.20.50
192.168.20.101
```

## 5. 미사용 IP 후보 검색

먼저 nmap에 응답한 IP를 파일로 저장합니다.

```bash
sudo nmap -sn -n 192.168.20.0/24 -oG - \
    | awk '/Up$/{print $2}' \
    | sort -V > /tmp/used_ips.txt
```

응답하지 않은 IP 후보를 표시합니다.

```bash
for i in $(seq 2 254); do
    ip="192.168.20.$i"
    if ! grep -Fxq "$ip" /tmp/used_ips.txt; then
        echo "$ip"
    fi
done
```

> **중요:** 검색되지 않은 주소가 반드시 비어 있는 것은 아닙니다. 전원이 꺼져 있거나 응답을 차단하는 기기, DHCP 예약 주소는 나타나지 않을 수 있습니다. 공유기 관리자 페이지에서 DHCP 할당 범위와 예약 목록을 확인하고 고정 IP를 선정하세요.

## 6. 고정 IP 및 부팅 시 자동 연결 설정

`192.168.20.100` 사용이 가능하고 **연결 프로필 이름이 `hyundai_02`인 것을 확인한 경우** (다른 이름이면 해당 프로필 이름으로 변경):

```bash
sudo nmcli connection modify 'hyundai_02' \
    ipv4.method manual \
    ipv4.addresses '192.168.20.100/24' \
    ipv4.gateway '192.168.20.1' \
    ipv4.dns '8.8.8.8 1.1.1.1' \
    connection.autoconnect yes
```

설정 적용:

```bash
sudo nmcli connection up 'hyundai_02'
```

> Wi-Fi로 SSH에 접속한 상태에서 IP를 변경하면 연결이 끊길 수 있습니다. 이후 새 IP로 재접속하세요.

## 7. 부팅 시 NetworkManager 및 SSH 자동 실행

```bash
sudo systemctl enable --now NetworkManager
sudo systemctl enable --now ssh
```

부팅 자동 실행 여부 확인:

```bash
systemctl is-enabled NetworkManager
systemctl is-enabled ssh
```

재부팅:

```bash
sudo reboot
```

## 8. 재부팅 후 확인

라즈베리파이에서:

```bash
ip -4 addr show wlan0
nmcli connection show --active
ip route
```

Windows 11 PowerShell에서:

```powershell
ssh 사용자명@192.168.20.100
```

`사용자명`은 Raspberry Pi OS 설치 시 만든 계정으로 변경합니다.

---

**최종 동작:** 전원을 켜면 `hyundai_02` Wi-Fi에 자동으로 연결하고, `192.168.20.100` 고정 IP로 SSH에 접속할 수 있습니다(네트워크 설정과 IP 충돌 여부 확인 필요).
