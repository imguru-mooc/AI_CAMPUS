# MCU Day 1 강의 자료 — ESP32 빌드 과정과 디지털 I/O

**온디바이스 AI · MCU 실습(ESP32) 1/4**

| 교시 | 주제 | 애니메이션 (anim/index.html) |
|---|---|---|
| 1 | 오리엔테이션 · ESP32 구조 | D1-1 핀맵 · 칩 구조 |
| 2 | 개발 환경과 빌드 과정 — 설치·빌드 따라하기 | D1-2 설치 · USB 연결 · 빌드 |
| 3 | Digital Output — LED 와 신호등 | D1-3 따라 꽂기 · LED · 신호등 |
| 4 | 레지스터 직접 제어 | D1-4 W1TS / W1TC 비트 뷰어 |
| 5 | Digital Input — 풀업과 디바운스 | D1-5 채터링과 디바운스 |
| 6 | Interrupt — ISR 과 공유 변수 | D1-6 인터럽트 타임라인 |
| 7 | PWM (LEDC) — 밝기 · 색 · 소리 | D1-7 듀티비 시각화 |
| 8 | 미니 프로젝트 — 반응속도 게임 · 정리 | D1-8 함수 포인터 상태 머신 |

### 이 자료 사용법

- **▶ 예제**는 스케치를 만들어 업로드하고, 시리얼 모니터(115200 baud) 출력과 LED·부저 동작을 함께 확인합니다. 주석의 `// →` 뒤가 기대 출력입니다.
- **✏️ 빈칸 채우기**는 `____` 를 채워 업로드합니다.
- **✏️ 괄호 넣기**는 ( ) 에 들어갈 말을 먼저 생각한 뒤 정답을 펼쳐 확인합니다.
- **🔒 정답 보기**는 접혀 있습니다. 강사가 신호를 준 뒤 펼치세요.
- **⚠ 함정** 표시는 오늘 반드시 한 번은 직접 밟아 봐야 하는 실수입니다. 오늘 함정은 대부분 **"아무 일도 일어나지 않음"** 으로 끝납니다 — LED 가 안 켜지면 코드보다 배선을 먼저 의심하세요.
- C 과정에서 배운 `enum`, 비트 연산, 구조체, 함수 포인터를 오늘 전부 **실제 핀**에 연결합니다.

---

## ⚙️ 준비 — 가장 먼저 확인

처음 설치하는 경우는 **2교시 따라하기 A (개발 환경 설치)** 를 순서대로 진행합니다. 아래 표는 설치를 마친 뒤 확인용입니다.

| 항목 | 확인 방법 |
|---|---|
| Arduino IDE 2.x | 도움말 → 정보 |
| ESP32 보드 패키지 (core 3.x) | 도구 → 보드 → 보드 매니저 → `esp32` by Espressif, **강사가 지정한 버전** |
| 보드 선택 | 도구 → 보드 → esp32 → **ESP32 Dev Module** |
| USB 드라이버 | 보드를 꽂았을 때 포트 목록에 `COMx` (Windows) 또는 `/dev/ttyUSB0` (Linux) 가 새로 생김 — CP2102 칩 |
| 케이블 | 포트가 안 생기면 **충전 전용 케이블**일 가능성이 가장 높음 |

**Day 1 공통 핀 배정** — 오늘 모든 예제가 이 배정을 씁니다. 아침에 **한 번만 꽂으면** 3교시부터 8교시까지 배선을 바꿀 필요가 없습니다 (7교시 RGB LED 만 예외).

| 부품 | GPIO | 브레드보드 위치 | 선 색 (그림 1) |
|---|---|---|---|
| LED 빨강 | 25 | 선 a4 · 저항 e4–f4 · LED g4(+) g5(−) · j5 → − 레일 | 주황 |
| LED 노랑 | 26 | 선 a8 · 저항 e8–f8 · LED g8(+) g9(−) · j9 → − 레일 | 노랑 |
| LED 초록 | 27 | 선 a12 · 저항 e12–f12 · LED g12(+) g13(−) · j13 → − 레일 | 초록 |
| Ball Tilt Sensor | 17 | 선 a18 · 센서 e18–f18 · j18 → − 레일 | 보라 |
| 버튼 | 4 | 선 a23 · 버튼 e23 e25 f23 f25 · j25 → − 레일 (대각선) | 파랑 |
| Active Buzzer | 19 | 선 a31 · 부저 e31(+) f31(−) · j31 → − 레일 | 회색 |
| Passive Buzzer | 23 | 선 a37 · 부저 e37(+) f37(−) · j37 → − 레일 | 분홍 |
| GND | GND | ESP32 오른쪽 맨 위 GND → 아래 파란 − 레일 1열 | 검정 |
| RGB LED (7교시) | 25 / 26 / 27 | 그림 4 참고 — LED 3개의 선을 옮겨 꽂음 | |

### 🔌 배선도 — 보고 그대로 꽂기

🎬 **D1-3 · 브레드보드 따라 꽂기** 탭에서 아래 순서를 한 단계씩 보며 꽂을 수 있고, 구멍을 누르면 속에서 이어진 칸이 표시됩니다.

<img src="img/wiring_day1_full.svg" alt="그림 1 Day 1 기본 배선">

**규칙은 하나입니다.** 모든 부품이 **"GPIO 선은 위쪽 a행 → 부품이 가운데 홈을 건넘 → 아래쪽 j행에서 파란 − 레일(GND)"** 모양입니다. 하나를 이해하면 나머지는 열 번호만 다릅니다.

ESP32 는 38핀이라 브레드보드에 꽂으면 한쪽 열만 남으므로, **보드는 브레드보드 옆에 두고 M-F 점퍼선**(한쪽이 구멍, 한쪽이 핀)으로 연결합니다. 구멍 쪽을 ESP32 핀에, 핀 쪽을 브레드보드에 꽂습니다.

#### 따라 꽂기 — 순서대로

1. **USB 가 뽑혀 있는지** 확인합니다.
2. 브레드보드를 **숫자 1 이 왼쪽, a행이 위쪽**이 되게 놓습니다. 파란 선(−)이 있는 레일이 **아래쪽**에 오도록 둡니다.
3. **저항 220Ω 3개** (빨강 빨강 갈색 띠) 의 다리를 ㄷ자로 구부려 가운데 홈을 건너 꽂습니다: `e4–f4`, `e8–f8`, `e12–f12`. 저항은 방향이 없습니다.
4. **LED 3개**를 꽂습니다. **긴 다리(+)가 왼쪽**: 빨강 `g4 · g5`, 노랑 `g8 · g9`, 초록 `g12 · g13`.
5. **기울기 센서**는 다리를 살짝 벌려 `e18–f18` 에 꽂습니다 (방향 없음).
6. **버튼**은 가운데 홈을 건너 `e23 · e25 · f23 · f25` 에 **딸깍 소리가 날 때까지** 눌러 꽂습니다.
7. **Active 부저**(뒷면 막힘)는 `+` 표시가 **위쪽**으로 `e31 (+) – f31`, **Passive 부저**(뒷면 기판 보임)는 `e37 (+) – f37` 에 꽂습니다.
8. **짧은 점퍼선(M-M) 7개**로 아래쪽 j행을 파란 − 레일에 잇습니다: `j5 · j9 · j13 · j18 · j25 · j31 · j37` → 바로 아래 − 레일.
9. **M-F 점퍼선**으로 ESP32 와 브레드보드를 잇습니다. 색을 맞춰 두면 나중에 찾기 쉽습니다.

| ESP32 핀 (보드 표기) | 위치 | → 브레드보드 |
|---|---|---|
| 25 | 왼쪽 줄 위에서 9번째 | a4 |
| 26 | 왼쪽 줄 위에서 10번째 | a8 |
| 27 | 왼쪽 줄 위에서 11번째 | a12 |
| 17 | 오른쪽 줄 위에서 11번째 | a18 |
| 4 | 오른쪽 줄 위에서 13번째 | a23 |
| 19 | 오른쪽 줄 위에서 8번째 | a31 |
| 23 | 오른쪽 줄 위에서 2번째 | a37 |
| GND | 오른쪽 줄 맨 위 | 아래 − 레일 1열 |

**10. 꽂은 뒤 점검** — 아래 네 가지를 확인하고 나서 USB 를 연결합니다.

- [ ] 어떤 선도 **3V3 · 5V 핀**에 꽂혀 있지 않다
- [ ] LED 긴 다리가 **저항 쪽(왼쪽)** 이다
- [ ] 모든 j행 점퍼가 **파란 −** 레일에 들어갔다 (빨간 + 레일 아님)
- [ ] ESP32 핀 번호를 보드의 **인쇄 글자**로 다시 읽어 확인했다 (위에서 몇 번째인지 세다 틀리는 경우가 많음)

> **레일이 중간에 끊긴 브레드보드** — 830홀 브레드보드 중에는 위아래 레일의 빨강·파랑 선이 **가운데(30열 근처)에서 끊겨** 있는 제품이 있습니다. 이 경우 왼쪽 − 레일과 오른쪽 − 레일을 짧은 점퍼선 하나로 이어 줘야 31 · 37열 부저가 GND 에 연결됩니다.

#### 브레드보드 속은 어떻게 이어져 있나

<img src="img/wiring_led_zoom.svg" alt="그림 2 LED 한 개 확대">

같은 **열 번호의 a~e 다섯 칸**, **f~j 다섯 칸**이 각각 속에서 이어져 있고, 가운데 홈을 경계로 **위아래는 끊겨** 있습니다. 그래서 저항이 홈을 건너야 위(GPIO)와 아래(LED)가 이어집니다. 레일은 **가로 한 줄 전체**가 이어져 있습니다.

<img src="img/wiring_button_zoom.svg" alt="그림 3 버튼 대각선 연결">

버튼은 다리 4개 중 **두 쌍이 원래 붙어 있고**, 누르면 두 쌍이 이어집니다. 어느 쌍이 붙어 있는지는 버튼 방향에 따라 달라서 헷갈리기 쉬우므로, **대각선 두 다리(a23 쪽 · j25 쪽)** 를 쓰면 방향과 상관없이 동작합니다.

**전압 안전 3원칙**

1. ESP32 핀은 **3.3V** 입니다. 5V 를 GPIO 에 직접 넣지 않습니다.
2. 핀 하나에서 뽑는 전류는 **20mA 이하**로 유지합니다 (LED 에는 항상 저항).
3. 배선을 바꿀 때는 **USB 를 뽑고** 바꿉니다.

---

# 1교시 · 오리엔테이션 · ESP32 구조 (09:00–09:50)

**학습 목표**
- ESP32 칩과 DevKit 보드의 구성 요소를 설명한다
- 38개 핀을 전원 / 일반 GPIO / 입력 전용 / Strapping / Flash 로 분류한다
- 3.3V 로직과 핀 전류 한계를 지키는 이유를 안다

🎬 **D1-1 핀맵 · 칩 구조** — 칩 블록을 눌러 역할과 쓰는 날 확인(칩 → 모듈 → 보드 포함), 보드의 핀을 눌러 분류 · 주의사항 · 브레드보드 위치 확인

### ▶ 예제 1-1 · 보드 위에 무엇이 있나

| 부품 | 역할 |
|---|---|
| ESP-32U 모듈 (금속 캔) | ESP32 칩 + 4MB Flash + 안테나 커넥터(U.FL). Xtensa LX6 **듀얼코어 240MHz**, SRAM 520KB, Wi-Fi · Bluetooth |
| CP2102 (정사각형 칩) | USB ↔ UART 변환. PC 의 `COMx` 포트가 바로 이 칩 |
| 레귤레이터 (3핀 칩) | USB 5V → 3.3V |
| EN 버튼 | 리셋 (칩을 다시 시작) |
| Boot 버튼 | 누른 채 리셋하면 **다운로드 모드** (GPIO 0 을 LOW 로) |
| Micro USB | 전원 + 업로드 + 시리얼 모니터 |

PC 의 C 프로그램은 OS 위에서 돌지만, 오늘의 프로그램은 **OS 없이** 이 칩 하나에서 돕니다. `printf` 가 화면 대신 **UART → CP2102 → USB → PC** 경로로 나간다는 것이 가장 큰 차이입니다.

### ▶ 예제 1-2 · 핀 분류표 — 오늘부터 매일 보는 표

| 분류 | 핀 | 규칙 |
|---|---|---|
| 전원 | 3V3, 5V, GND | 3V3 은 출력용 전원. 5V 는 USB 전원이 그대로 나옴 |
| 일반 GPIO (출력 가능) | 4, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33 | 오늘 쓰는 핀은 전부 여기서 고름 |
| 입력 전용 | 34, 35, 36(VP), 39(VN) | 출력 불가, **내부 풀업 없음**. 아날로그 센서 전용 (Day 2) |
| Strapping | 0, 2, 5, 12, 15 | 부팅 순간의 전압으로 동작 모드가 정해짐. **12 는 피한다** |
| Flash 연결 | 6, 7, 8, 9, 10, 11 (보드 표기 CLK, D0~D3, CMD) | **절대 사용 금지** — 연결하면 부팅 불가 |
| UART0 | 1(TX), 3(RX) | 업로드와 시리얼 모니터가 사용 중 |

### ▶ 예제 1-3 · ⚠ 함정 — "핀이 있으니 쓸 수 있겠지"

| 한 일 | 결과 |
|---|---|
| GPIO 34 에 LED 연결 후 `digitalWrite(34, HIGH)` | 아무 일도 없음 — 입력 전용 핀 |
| GPIO 9 (보드 표기 D2) 에 부품 연결 | 부팅 실패 또는 무한 리셋 — 내부 Flash 선을 건드림 |
| GPIO 12 에 풀업 부품 연결 | 부팅 시 Flash 전압이 잘못 잡혀 업로드·부팅 실패 가능 |
| 5V 센서 출력을 GPIO 에 직접 | 당장은 동작해도 핀이 손상될 수 있음 → Day 2 에서 **분압**으로 해결 |
| 38핀 보드를 830홀 브레드보드 가운데에 꽂음 | 한쪽 열만 남아 반대쪽 핀에 선을 꽂을 곳이 없음 → 보드를 **한쪽 끝**에 걸치거나 M-F 점퍼선 사용 |

오늘 키트의 **GPIO Extension Board 와 40핀 케이블은 Raspberry Pi 전용**이라 쓰지 않습니다.

#### ✏️ 빈칸 채우기 1-1

핀 번호를 받아 "출력으로 써도 안전한가"를 판단하는 함수입니다. 2교시에 업로드해 확인합니다.

```c
const int FLASH_PINS[] = {6, 7, 8, 9, 10, 11};
const int STRAP_PINS[] = {0, 2, 5, 12, 15};

int in_list(int pin, const int *list, int n) {
    for (int i = 0; i < n; i++)
        if (list[i] == pin) return 1;
    return 0;
}

int is_safe_output(int pin) {
    if (pin >= ____ && pin <= 39) return 0;             // 입력 전용
    if (in_list(pin, FLASH_PINS, 6)) return 0;          // Flash
    if (in_list(pin, STRAP_PINS, ____)) return 0;       // Strapping
    if (pin == 1 || pin == ____) return 0;              // UART0 TX/RX
    return 1;
}

void setup() {
    Serial.begin(115200);
    int test[] = {25, 34, 9, 12, 4};
    for (int i = 0; i < 5; i++)
        Serial.printf("GPIO %2d -> %s\n", test[i], is_safe_output(test[i]) ? "OK" : "NO");
    // → GPIO 25 -> OK / 34 -> NO / 9 -> NO / 12 -> NO / 4 -> OK
}

void loop() {}
```

<details><summary>🔒 정답 보기 1-1</summary>

```c
    if (pin >= 34 && pin <= 39) return 0;
    if (in_list(pin, STRAP_PINS, 5)) return 0;
    if (pin == 1 || pin == 3) return 0;
```

`const int *list` 로 배열을 받고 **길이 n 을 따로 넘기는** 모양은 C 과정 Day 2 의 "배열은 함수에 넘기는 순간 포인터가 된다" 그대로입니다.
</details>

#### ✏️ 괄호 넣기 1-2

1. ESP32 의 CPU 는 (　　　)코어이며 최대 (　　　)MHz 로 동작한다.
2. PC 와 USB 로 통신하게 해 주는 보드 위의 변환 칩은 (　　　)이다.
3. GPIO 34~39 는 (　　　) 전용 핀이며 내부 풀업이 (`있다` / `없다`).
4. 보드 표기 CLK, D0~D3, CMD 핀은 내부 (　　　)에 연결되어 있어 사용하면 안 된다.
5. ESP32 GPIO 의 논리 전압은 (　　　)V 이다.

<details><summary>🔒 정답 보기 1-2</summary>

1. 듀얼 / 240
2. CP2102
3. 입력 / 없다
4. Flash
5. 3.3
</details>

---

# 2교시 · 개발 환경과 빌드 과정 (10:00–10:50)

**학습 목표**
- Arduino IDE 와 ESP32 코어를 설치하고 보드·포트를 설정한다
- 새 스케치 → 저장 → 컴파일 → 업로드 → 시리얼 모니터 순서를 손에 익힌다
- 첫 스케치를 업로드하고 시리얼 모니터로 결과를 확인한다
- `.ino` 가 `.bin` 이 되어 Flash 에 기록되기까지의 단계를 설명한다
- `setup()` 과 `loop()` 를 누가 호출하는지 안다

🎬 **D1-2 설치 · USB 연결 · 빌드** — 설치 체크리스트, Micro USB 방향 맞춰 꽂기, 빌드 단계를 하나씩 진행하며 파일과 명령 확인

> **운영 팁** — 설치(따라하기 A)는 다운로드에 시간이 걸리므로 **교육 전날 사전 설치**를 권장합니다. 사전 설치를 마친 수강생은 수업에서 **A-5 (Micro USB 연결) 부터** 확인만 하면 됩니다. 교육장 네트워크가 느리면 강사가 준비한 오프라인 패키지(따라하기 D)를 씁니다.

### 🛠 따라하기 A · 개발 환경 설치 (처음 한 번)

#### A-1 · Arduino IDE 2.x 다운로드와 설치

1. 브라우저에서 `https://www.arduino.cc/en/software` 에 접속합니다.
2. **Arduino IDE 2.x** 항목에서 운영체제에 맞는 파일을 고릅니다. (기부 화면이 나오면 **JUST DOWNLOAD** 를 누르면 됩니다.)

| 운영체제 | 받을 파일 | 설치 |
|---|---|---|
| Windows 10/11 | Windows **Win 10 and newer, 64 bits** (`.exe`) | 실행 → 동의 → "모든 사용자" 또는 "현재 사용자" → 설치 경로 기본값 → 설치. 중간에 **USB 드라이버 설치 창이 뜨면 모두 "설치"** |
| macOS | Apple Silicon 또는 Intel (`.dmg`) | 열어서 Arduino IDE 아이콘을 **Applications** 폴더로 끌어 놓기 |
| Linux | AppImage 64 bits | `chmod +x arduino-ide_*.AppImage` 후 실행 |

3. Arduino IDE 를 처음 실행하면 추가 구성요소를 내려받느라 **1~2분** 걸립니다. 오른쪽 아래 진행 표시가 사라질 때까지 기다립니다.

#### A-2 · 한국어 메뉴와 스케치북 위치

1. **File → Preferences** (한국어 메뉴에서는 **파일 → 기본 설정**, 단축키 `Ctrl + ,`) 를 엽니다.
2. **Language(언어)** 를 `한국어` 로 바꾸면 IDE 가 다시 시작됩니다. 이후 메뉴 이름은 한국어 기준으로 적습니다.
3. **스케치북 위치**를 확인합니다. 기본값은 `문서\Arduino` 입니다.

| 확인 | 조치 |
|---|---|
| 경로에 한글이나 공백이 없다 (`C:\Users\hong\Documents\Arduino`) | 그대로 사용 |
| 경로에 한글이 있다 (`C:\Users\홍길동\Documents\Arduino`) | **찾아보기**를 눌러 `C:\esp32` 폴더를 만들어 지정 — 한글 경로는 빌드 오류의 가장 흔한 원인 |

4. 같은 창에서 **"다음 동안 자세한 출력 보이기"** 의 **컴파일**, **업로드** 두 칸을 모두 체크합니다. 예제 2-2 에서 빌드 로그를 읽을 때 필요합니다.

#### A-3 · ESP32 보드 매니저 URL 추가

1. 같은 **기본 설정** 창 아래쪽 **추가 보드 관리자 URL** 오른쪽의 창 아이콘을 누릅니다.
2. 아래 주소를 한 줄로 붙여 넣고 **확인 → 확인** 을 누릅니다.

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

이미 다른 URL 이 있으면 **새 줄**에 추가합니다. 이 주소는 Espressif 가 배포하는 ESP32 보드 패키지 목록입니다.

#### A-4 · esp32 코어 설치

1. 왼쪽 세로 막대의 **보드 매니저** 아이콘(두 번째 아이콘)을 누르거나 **도구 → 보드 → 보드 매니저** 를 엽니다.
2. 검색 칸에 `esp32` 를 입력합니다.
3. **esp32 by Espressif Systems** 를 찾습니다. (비슷한 이름의 "Arduino ESP32 Boards" 는 Arduino Nano ESP32 전용이므로 **고르지 않습니다.**)
4. 버전 목록에서 **강사가 지정한 3.x 버전**을 고르고 **설치** 를 누릅니다.
5. 컴파일러(xtensa 툴체인)와 esptool 등 수백 MB 를 내려받습니다. 출력 창 마지막에 `Platform esp32:esp32@3.x.x installed` 가 나오면 완료입니다.

| 증상 | 조치 |
|---|---|
| 검색해도 esp32 by Espressif 가 없음 | A-3 의 URL 오타 확인 → IDE 재시작 |
| 다운로드 중 `timeout`, `network error` | 교육장 방화벽 또는 느린 네트워크 → 다시 설치 누르기, 계속 실패하면 따라하기 D |
| 버전이 3.x 가 아니라 2.x 로 설치됨 | 1교시 이후 예제의 `ledcAttach` 가 컴파일되지 않음 → **제거 후 3.x 로 재설치** |

#### A-5 · 보드를 Micro USB 로 연결하기

🎬 **D1-2 · 설치 체크 · Micro USB** 탭에서 플러그를 뒤집어 가며 먼저 연습해 봅니다.

오늘 보드는 **Micro USB (Micro-B)** 단자입니다. 스마트폰에 흔한 USB-C 와 모양이 다르고, **위아래 방향이 정해져 있습니다.** 억지로 꽂으면 단자가 보드에서 떨어져 나갈 수 있으니 아래 순서를 지킵니다.

**① 케이블 준비**

| 확인 | 설명 |
|---|---|
| 보드 쪽 끝 | **Micro-B** (사다리꼴 모양, 폭 약 7mm) |
| PC 쪽 끝 | USB-A (넓은 직사각형) 또는 USB-C. 노트북에 USB-C 포트만 있으면 **USB-C ↔ Micro-B 케이블**이나 USB-C → USB-A 어댑터를 씁니다 |
| 데이터 케이블인가 | 겉모양으로는 구분이 어렵습니다. **꽂았을 때 포트가 생기면 데이터 케이블**(A-6 에서 확인). 보조배터리·선풍기에 딸려 온 케이블은 충전 전용인 경우가 많습니다 |
| 길이 | 1m 이하 권장. 너무 길거나 얇은 케이블은 업로드가 자주 끊깁니다 |

**② 보드 준비**

1. 보드 핀에 꽂혀 온 **포장용 스펀지를 떼어 냅니다.** (흰색·검은색 스펀지에 핀이 꽂힌 채 배송되는 경우가 많습니다)
2. 보드를 **금속 책상이나 전선 위에 올려 두지 않습니다.** 핀끼리 닿아 합선될 수 있습니다. 브레드보드에 꽂거나 종이·스펀지 없는 비전도성 표면에 둡니다.
3. 오늘 실습 배선(LED, 버튼 등)이 있다면 **배선을 먼저 끝내고 USB 는 마지막에** 연결합니다.

**③ 방향 맞추기 — 사다리꼴을 맞춘다**

<div markdown="0" style="display:flex;gap:28px;align-items:center;flex-wrap:wrap;margin:10px 0 14px">
<svg viewBox="0 0 520 170" width="520" style="max-width:100%;height:auto;background:#fff;border:1px solid #C9D1DC;border-radius:8px" role="img" aria-label="Micro-B 단자와 플러그 방향 맞추기">
  <text x="120" y="24" text-anchor="middle" font-size="13" fill="#1B2A41" font-family="sans-serif">보드의 단자 (정면에서 본 모양)</text>
  <rect x="40" y="46" width="160" height="80" rx="6" fill="#1B2A41"/>
  <path d="M78,68 L162,68 L150,100 L90,100 Z" fill="#DCE6FA" stroke="#9EA4AC" stroke-width="3"/>
  <text x="120" y="62" text-anchor="middle" font-size="11" fill="#FFFFFF" font-family="sans-serif">넓은 쪽</text>
  <text x="120" y="118" text-anchor="middle" font-size="11" fill="#FFFFFF" font-family="sans-serif">좁은 쪽</text>
  <text x="120" y="150" text-anchor="middle" font-size="11" fill="#5B6B80" font-family="sans-serif">보드 기판(PCB) 쪽</text>
  <text x="260" y="92" text-anchor="middle" font-size="26" fill="#178A67" font-family="sans-serif">⇐</text>
  <text x="400" y="24" text-anchor="middle" font-size="13" fill="#1B2A41" font-family="sans-serif">케이블 플러그 (끝에서 본 모양)</text>
  <rect x="330" y="50" width="140" height="72" rx="8" fill="#5B6B80"/>
  <path d="M358,68 L442,68 L430,100 L370,100 Z" fill="#C9D1DC" stroke="#1B2A41" stroke-width="3"/>
  <text x="400" y="62" text-anchor="middle" font-size="11" fill="#FFFFFF" font-family="sans-serif">넓은 쪽</text>
  <text x="400" y="118" text-anchor="middle" font-size="11" fill="#FFFFFF" font-family="sans-serif">좁은 쪽</text>
  <text x="400" y="150" text-anchor="middle" font-size="11" fill="#178A67" font-family="sans-serif">넓은 쪽끼리, 좁은 쪽끼리 맞춘다</text>
</svg>
</div>

1. 보드의 USB 단자 구멍을 **정면에서** 봅니다. 구멍이 **한쪽이 넓고 한쪽이 좁은 사다리꼴**입니다.
2. 케이블 플러그 끝을 봅니다. 플러그도 같은 사다리꼴입니다.
3. **넓은 쪽은 넓은 쪽끼리** 맞춥니다. 대부분의 DevKit 보드는 단자의 넓은 쪽이 **위(기판 반대쪽)** 입니다.

**④ 꽂기**

1. 한 손으로 **보드(USB 단자 옆)를 잡아 고정**합니다. 브레드보드에 꽂혀 있으면 브레드보드째 잡습니다.
2. 다른 손으로 플러그를 **수평으로 곧게** 밀어 넣습니다. 끝까지 들어가면 가볍게 딸깍 걸리는 느낌이 납니다.
3. **들어가지 않으면 힘을 주지 말고** 플러그를 뒤집어 다시 맞춥니다. 방향이 맞으면 거의 힘이 들지 않습니다.
4. PC 쪽 끝을 PC 본체의 USB 포트에 꽂습니다. 가능하면 **키보드·모니터의 USB 포트나 전원 없는 USB 허브는 피하고** PC 에 직접 꽂습니다(전류 부족과 업로드 끊김 방지).

**⑤ 연결 확인**

| 보이는 것 | 의미 | 조치 |
|---|---|---|
| 보드의 **빨간 전원 LED** 가 켜짐 | 전원 정상 | A-6 에서 포트 확인 |
| LED 가 켜지지 않음 | 플러그가 덜 들어감, 케이블 불량, PC 포트 문제 | 끝까지 다시 꽂기 → 다른 PC 포트 → 다른 케이블 |
| 꽂는 순간 PC 에서 "USB 장치 전원 서지" 경고 · 보드가 뜨거움 | 배선 합선 가능성 | **즉시 USB 를 뽑고** 배선(특히 3V3·5V 와 GND 가 붙어 있는지) 확인 |
| 만질 때마다 연결이 끊겼다 붙었다 함 | 헐거운 단자·케이블 | 케이블 교체. 계속되면 단자 납땜 불량일 수 있으니 강사에게 보고 |

**⑥ 뽑을 때와 다루기 주의**

- 뽑을 때는 **케이블 줄이 아니라 플러그 몸통을 잡고 수평으로** 빼냅니다. 위아래로 비틀면 단자가 기판에서 떨어집니다.
- 꽂은 상태로 케이블에 보드가 **매달리거나 당겨지지 않게** 케이블을 책상 위에 느슨하게 둡니다.
- **배선을 바꿀 때는 먼저 USB 를 뽑습니다** (전압 안전 3원칙 3번). 업로드가 끝나도 보드는 계속 전원이 들어와 있습니다.
- 노트북이 절전 모드로 들어가면 USB 전원이 끊겨 포트가 사라질 수 있습니다. 실습 중에는 절전을 끄거나, 포트가 사라지면 다시 꽂고 **도구 → 포트**를 다시 고릅니다.

#### A-6 · USB 드라이버와 포트 확인 (CP2102)

A-5 대로 보드를 연결한 상태에서 PC 가 보드를 인식했는지 확인합니다.

**Windows**

1. 시작 버튼 우클릭 → **장치 관리자** 를 엽니다.
2. **포트(COM & LPT)** 를 펼칩니다.

| 보이는 것 | 의미 | 조치 |
|---|---|---|
| `Silicon Labs CP210x USB to UART Bridge (COM5)` | 정상. 숫자(COM5)를 기억 | 없음 |
| **기타 장치** 아래 `CP2102 USB to UART Bridge Controller` (노란 느낌표) | 드라이버 없음 | 아래 드라이버 설치 |
| 아무 변화 없음 | 케이블이 **충전 전용**이거나 USB 포트 문제 | 다른 케이블, 다른 USB 포트(가능하면 PC 뒷면) |

드라이버 설치: 브라우저에서 `https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers` → **Downloads** 탭 → **CP210x Universal Windows Driver** 를 받습니다. 압축을 풀고 `silabser.inf` 를 **우클릭 → 설치** 한 뒤 보드를 뽑았다가 다시 꽂습니다.

**macOS**

터미널에서 확인합니다. `/dev/cu.usbserial-XXXX` 또는 `/dev/cu.SLAB_USBtoUART` 가 보이면 정상입니다.

```bash
ls /dev/cu.*
```

**Linux (Ubuntu)**

```bash
ls /dev/ttyUSB*                     # → /dev/ttyUSB0
sudo usermod -aG dialout $USER      # 포트 접근 권한 (한 번만)
# 로그아웃 후 다시 로그인해야 적용됨
```

보드를 꽂자마자 `/dev/ttyUSB0` 가 사라지면 점자 디스플레이 서비스가 포트를 가져가는 경우입니다: `sudo apt remove brltty`

#### A-7 · 보드와 포트 선택

1. **도구 → 보드 → esp32 → ESP32 Dev Module** 을 고릅니다. (목록이 길면 IDE 위쪽 보드 선택 칸에서 `ESP32 Dev` 로 검색)
2. **도구 → 포트** 에서 A-6 에서 확인한 포트(`COM5`, `/dev/cu.usbserial-…`, `/dev/ttyUSB0`)를 고릅니다.
3. **도구** 메뉴의 나머지 항목이 아래와 같은지 확인합니다. 보드를 고르면 대부분 기본값이 이렇게 잡힙니다.

| 도구 메뉴 항목 | 값 |
|---|---|
| Upload Speed | 921600 (업로드가 자꾸 끊기면 115200) |
| CPU Frequency | 240MHz (WiFi/BT) |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Default 4MB with spiffs |
| Core Debug Level | None (Day 4 디버깅 교시에서 변경) |
| Erase All Flash Before Sketch Upload | Disabled |

#### A-8 · 설치 확인 체크리스트

- [ ] IDE 메뉴가 한국어로 나오고 스케치북 경로에 한글이 없다
- [ ] 보드 매니저에 `esp32 by Espressif Systems 3.x.x INSTALLED` 가 보인다
- [ ] Micro USB 를 사다리꼴 방향에 맞춰 힘들이지 않고 꽂았고, 빨간 전원 LED 가 켜진다
- [ ] 보드를 꽂으면 포트가 생기고, 뽑으면 사라진다
- [ ] 도구 → 보드가 **ESP32 Dev Module**, 도구 → 포트가 방금 확인한 포트다

다섯 칸이 모두 체크되면 따라하기 B 로 넘어갑니다.

### 🛠 따라하기 B · 새 스케치 만들기부터 업로드까지

오늘부터 모든 예제를 이 순서로 실행합니다. 단축키를 익혀 두면 하루가 편해집니다.

| 단계 | 메뉴 | 단축키 (Windows · Linux / macOS) | 하는 일 |
|---|---|---|---|
| 1 | 파일 → 새 스케치 | `Ctrl+N` / `⌘N` | 빈 스케치 창 |
| 2 | 파일 → 저장 | `Ctrl+S` / `⌘S` | 스케치 이름 정하기 |
| 3 | 스케치 → 확인/컴파일 (✓ 버튼) | `Ctrl+R` / `⌘R` | 빌드만 (보드 불필요) |
| 4 | 스케치 → 업로드 (→ 버튼) | `Ctrl+U` / `⌘U` | 빌드 + Flash 에 쓰기 |
| 5 | 도구 → 시리얼 모니터 | `Ctrl+Shift+M` / `⌘⇧M` | 보드 출력 보기 |

#### B-1 · 새 스케치와 저장 규칙

1. `Ctrl+N` 으로 새 스케치를 엽니다. `void setup()` 과 `void loop()` 가 미리 들어 있습니다. **전부 지우고** 예제 코드를 붙여 넣습니다.
2. `Ctrl+S` 로 저장합니다. 이름은 **영문 소문자, 숫자, 밑줄**만 씁니다 (예: `hello_esp32`).
3. 저장하면 **스케치 이름과 같은 폴더**가 생기고 그 안에 `.ino` 파일이 들어갑니다. Arduino 는 폴더 이름과 파일 이름이 다르면 스케치를 열지 않습니다.

```text
C:\esp32\
 └─ hello_esp32\
     └─ hello_esp32.ino
```

오늘 예제는 `day1\` 폴더 아래에 예제 번호로 저장하기를 권합니다: `ex2_1_hello`, `ex3_3_traffic`, `ex8_reaction_game` …

#### B-2 · 컴파일(빌드)만 해 보기

1. **✓ (확인)** 버튼 또는 `Ctrl+R` 을 누릅니다. **보드가 연결되어 있지 않아도** 됩니다.
2. 아래 **출력** 창에 로그가 흐르고, 성공하면 마지막에 이런 줄이 나옵니다.

```text
Sketch uses 290841 bytes (22%) of program storage space. Maximum is 1310720 bytes.
Global variables use 20712 bytes (6%) of dynamic memory, leaving 306968 bytes for local variables. Maximum is 327680 bytes.
```

숫자는 예시이며 core 버전마다 다릅니다. 첫 컴파일은 core 전체를 빌드하느라 **1~3분**, 두 번째부터는 바뀐 파일만 빌드해 몇 초면 끝납니다.

3. 오류가 나면 출력 창에서 **빨간 글자 중 가장 위의 `error:`** 를 찾습니다. 형식은 C 과정의 gcc 와 같습니다.

```text
C:\esp32\hello_esp32\hello_esp32.ino: In function 'void setup()':
C:\esp32\hello_esp32\hello_esp32.ino:4:5: error: 'Serail' was not declared in this scope
```

`파일:줄:칸: error:` — 4번째 줄 5번째 칸에서 `Serial` 을 `Serail` 로 잘못 썼다는 뜻입니다.

#### B-3 · 업로드

1. 보드를 연결하고 **도구 → 포트**가 맞는지 확인합니다.
2. **→ (업로드)** 버튼 또는 `Ctrl+U` 를 누릅니다. 컴파일이 끝나면 곧바로 업로드가 시작됩니다.
3. 출력 창의 업로드 로그를 따라갑니다 (예시).

```text
esptool.py v4.x
Serial port COM5
Connecting....                           ← 여기서 멈추면 Boot 버튼 (아래 참고)
Chip is ESP32-D0WD-V3 (revision v3.1)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Uploading stub...
Changing baud rate to 921600
Writing at 0x00010000... (10 %)
Writing at 0x0001c3a2... (20 %)
...
Writing at 0x0004a1f0... (100 %)
Hash of data verified.
Leaving...
Hard resetting via RTS pin...            ← 끝. 보드가 새 프로그램으로 다시 시작
```

**Connecting 에서 멈출 때 — Boot 버튼 쓰는 법**

1. 업로드(→)를 누른다
2. 출력 창에 `Connecting....` 이 보이면 보드의 **Boot 버튼을 누르고 있는다**
3. `Writing at 0x...` 가 나오기 시작하면 **손을 뗀다**

#### B-4 · 시리얼 모니터로 결과 확인

1. `Ctrl+Shift+M` 또는 오른쪽 위 **돋보기 아이콘**을 누르면 아래쪽에 **시리얼 모니터** 탭이 열립니다.
2. 오른쪽의 속도 목록을 **115200 baud** 로 맞춥니다. (코드의 `Serial.begin(115200)` 과 같아야 함)
3. 아무것도 안 보이면 보드의 **EN 버튼**을 한 번 누릅니다. 부팅 로그부터 다시 출력됩니다.
4. 업로드할 때 IDE 가 포트를 잠시 가져가므로 시리얼 모니터는 끊겼다가 자동으로 다시 연결됩니다.

**시리얼 플로터**(도구 → 시리얼 플로터)는 숫자를 그래프로 그려 줍니다. Day 2 센서 실습에서 씁니다.

#### B-5 · 빌드 결과물(.bin) 직접 꺼내 보기

**스케치 → 컴파일된 바이너리 내보내기** (`Alt+Ctrl+S` / `⌥⌘S`) 를 누르면 스케치 폴더 안에 `build\` 폴더가 생깁니다.

```text
hello_esp32\
 ├─ hello_esp32.ino
 └─ build\esp32.esp32.esp32\
     ├─ hello_esp32.ino.bin              ← 0x10000 에 쓰이는 앱 이미지
     ├─ hello_esp32.ino.bootloader.bin   ← 0x1000
     ├─ hello_esp32.ino.partitions.bin   ← 0x8000
     ├─ hello_esp32.ino.elf              ← 링크 결과 (디버그 정보 포함)
     └─ hello_esp32.ino.map              ← 함수·변수가 메모리 어디에 놓였는지
```

`.elf` 가 `.bin` 보다 훨씬 큰 이유는 디버그 정보가 들어 있기 때문입니다. 예제 2-2 의 빌드 단계가 실제 파일로 남은 것입니다.

### 🛠 따라하기 C · (심화) 명령줄로 빌드하기 — arduino-cli

IDE 의 확인·업로드 버튼 뒤에서는 `arduino-cli` 와 같은 동작이 일어납니다. C 과정에서 `gcc` 를 터미널에서 썼던 것처럼 직접 실행해 봅니다. 시간이 남는 수강생용입니다.

1. `https://arduino.github.io/arduino-cli/` 의 **Installation** 에서 운영체제에 맞는 파일을 받아 압축을 풀고 `arduino-cli` 실행 파일을 PATH 에 둡니다.
2. 터미널에서 아래를 순서대로 실행합니다.

```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32@3.x.x        # 강사 지정 버전 번호로
arduino-cli board list                            # 연결된 포트 확인 → COM5 등

# 빌드: FQBN(esp32:esp32:esp32) = 패키지:아키텍처:보드(ESP32 Dev Module)
arduino-cli compile --fqbn esp32:esp32:esp32 --output-dir build hello_esp32

# 업로드 + 시리얼 모니터
arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 hello_esp32
arduino-cli monitor -p COM5 -c baudrate=115200    # 종료: Ctrl+C
```

`compile` 에 `-v` 를 붙이면 예제 2-2 의 xtensa-esp32-elf-g++ 명령이 그대로 보입니다. IDE 와 arduino-cli 는 같은 core 를 공유하므로 IDE 에서 이미 설치했다면 `core install` 은 바로 끝납니다.

### 🛠 따라하기 D · (강사용) 오프라인 설치 패키지

교육장 네트워크에서 여러 명이 동시에 core 를 받으면 실패하기 쉽습니다. 설치가 끝난 PC 한 대에서 아래 폴더를 통째로 복사해 USB 나 공유 폴더로 나눠 줍니다.

| 운영체제 | 복사할 폴더 (보드 패키지 · 툴체인) |
|---|---|
| Windows | `C:\Users\<사용자>\AppData\Local\Arduino15` |
| macOS | `~/Library/Arduino15` |
| Linux | `~/.arduino15` |

수강생 PC 에서는 Arduino IDE 를 설치하고 **한 번 실행했다가 종료**한 뒤, 위 경로에 복사한 폴더를 덮어씁니다. 다시 실행해 보드 매니저에 `esp32 ... INSTALLED` 가 보이면 완료입니다. 같은 운영체제끼리만 복사할 수 있습니다 (Windows 용 툴체인은 macOS 에서 동작하지 않음).

### ▶ 예제 2-1 · Hello ESP32 — 칩 정보 출력

`hello_esp32.ino`
```c
void setup() {
    Serial.begin(115200);               // PC 쪽 시리얼 모니터도 115200 으로
    delay(500);
    Serial.println("Hello ESP32");
    Serial.printf("Chip  : %s rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf("Cores : %d\n", ESP.getChipCores());               // → 2
    Serial.printf("CPU   : %lu MHz\n", ESP.getCpuFreqMHz());         // → 240
    Serial.printf("Flash : %lu bytes\n", ESP.getFlashChipSize());    // → 4194304
    Serial.printf("Heap  : %lu bytes free\n", ESP.getFreeHeap());
}

void loop() {
    Serial.printf("uptime %lu ms\n", millis());
    delay(1000);
}
```

따라하기 B 순서대로 `hello_esp32` 로 저장 → `Ctrl+R` 컴파일 → `Ctrl+U` 업로드 → `Ctrl+Shift+M` 시리얼 모니터(115200). 업로드 중 `Connecting.....` 에서 멈추면 **Boot 버튼을 누르고 있다가** 쓰기가 시작되면 뗍니다.

C 과정과 달리 `main()` 이 없고 `printf` 대신 `Serial.printf` 를 씁니다. 형식 지정자(`%d`, `%s`, `%lu`)는 똑같습니다.

### ▶ 예제 2-2 · 빌드 로그 읽기 — gcc 가 여기서도 돈다

따라하기 A-2 에서 **"자세한 출력 보이기: 컴파일, 업로드"** 를 체크했으면 다시 업로드합니다. 로그에서 아래 단계를 찾아 표시합니다 (경로는 PC 마다 다름).

| 단계 | 로그에서 찾을 것 | C 과정 대응 |
|---|---|---|
| 1. 스케치 전처리 | `.ino` → `hello_esp32.ino.cpp` (함수 원형 자동 추가) | 전처리 |
| 2. 컴파일 | `xtensa-esp32-elf-g++ ... -c ... -o hello_esp32.ino.cpp.o` | `gcc -c` |
| 3. 링크 | `... -o hello_esp32.ino.elf` (core 라이브러리와 결합) | 링크 |
| 4. 이미지 변환 | `esptool ... elf2image` → `hello_esp32.ino.bin` | (PC 에는 없음) |
| 5. 크기 보고 | `Sketch uses ... bytes (..%) of program storage space` | `size` 명령 |
| 6. 업로드 | `esptool ... write_flash 0x1000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 hello_esp32.ino.bin` | (PC 에는 없음) |

PC 에서는 `./app` 을 실행하면 OS 가 파일을 메모리에 올려 줍니다. MCU 에는 OS 가 없으므로 **Flash 의 정해진 주소**에 이미지를 직접 써 두고, 전원이 켜지면 ROM → bootloader(0x1000) → 파티션 테이블(0x8000) → 앱(0x10000) 순서로 시작합니다.

### ▶ 예제 2-3 · setup() 과 loop() 는 누가 부르나

Arduino-ESP32 core 내부(간략화):
```c
extern "C" void app_main() {               // ESP-IDF 의 진짜 시작점
    initArduino();
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, 1);  // core 1
}

void loopTask(void *arg) {
    setup();                               // 한 번
    for (;;) {
        loop();                            // 무한 반복
    }
}
```

직접 확인:
```c
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.printf("setup: task=%s core=%d\n", pcTaskGetName(NULL), xPortGetCoreID());
    // → setup: task=loopTask core=1
}

void loop() {}
```

`setup()`/`loop()` 는 **FreeRTOS 태스크 하나 안에서 돌아가는 평범한 함수**입니다. Day 4 에서 이 태스크 옆에 우리 태스크를 더 만듭니다.

### ▶ 예제 2-4 · ⚠ 함정 — 글자가 깨진다

시리얼 모니터 속도를 **9600** 으로 바꾸고 EN 버튼을 눌러 봅니다. 알아볼 수 없는 문자가 나옵니다. 다시 **115200** 으로 맞추고 EN 을 누르면 부팅 로그부터 보입니다.

```
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
...
Hello ESP32
```

양쪽 속도(baud)가 다르면 비트를 읽는 간격이 달라 **바이트가 엉뚱하게 조립**됩니다. 코드 문제가 아닙니다.

| 증상 | 조치 |
|---|---|
| 포트가 보이지 않음 | 데이터 케이블로 교체, CP2102 드라이버 설치 |
| `Failed to connect to ESP32` | "Connecting..." 동안 Boot 버튼을 누르고 있기 |
| 업로드 도중 끊김 | 도구 → Upload Speed → 115200 |
| 빌드 경로 오류 | Windows 계정명이 한글이면 스케치북 위치를 `C:\esp32\` 로 변경 |

#### ✏️ 빈칸 채우기 2-1

부팅 후 몇 번째 loop 인지와 경과 시간을 출력합니다.

```c
unsigned long count = 0;

void setup() {
    Serial.begin(____);
}

void loop() {
    count++;
    Serial.printf("loop #%lu at %lu ms\n", count, ____);
    ____(1000);
}
// → loop #1 at 0 ms / loop #2 at 1000 ms / ...
```

<details><summary>🔒 정답 보기 2-1</summary>

```c
    Serial.begin(115200);
    Serial.printf("loop #%lu at %lu ms\n", count, millis());
    delay(1000);
```

`millis()` 는 부팅 후 경과 시간(ms)을 `unsigned long` 으로 돌려줍니다. 약 49.7일 뒤 0 으로 돌아옵니다 (5교시에서 다시 만남).
</details>

#### ✏️ 괄호 넣기 2-2

1. ESP32 용 컴파일러의 이름은 (　　　)-esp32-elf-g++ 이다.
2. 링크 결과물은 (　　　) 파일이고, esptool 이 이를 (　　　) 파일로 변환해 업로드한다.
3. 앱 이미지는 Flash 의 (　　　) 주소에 기록된다.
4. `setup()` 과 `loop()` 는 (　　　)라는 이름의 FreeRTOS 태스크 안에서 호출된다.
5. 업로드가 `Connecting...` 에서 멈추면 (`EN` / `Boot`) 버튼을 누르고 있는다.

<details><summary>🔒 정답 보기 2-2</summary>

1. xtensa
2. .elf / .bin
3. 0x10000
4. loopTask
5. Boot
</details>

---

# 3교시 · Digital Output — LED 와 신호등 (11:00–11:50)

**학습 목표**
- 옴의 법칙으로 LED 전류 제한 저항을 계산한다
- `pinMode`, `digitalWrite` 로 여러 핀을 제어한다
- `enum` + `switch` 로 상태 기반 신호등을 만든다

🎬 **D1-3 따라 꽂기 · LED · 신호등** — 그림 1 을 한 단계씩 꽂고 구멍을 눌러 이어진 칸 확인, 저항값을 바꿔 전류 확인, 신호등 상태 전이 재생

### ▶ 예제 3-1 · 저항은 왜 220Ω 인가

LED 에 걸리는 전압은 색마다 거의 일정합니다(순방향 전압 Vf). 나머지 전압이 저항에 걸리고, 그 저항이 전류를 정합니다.

**I = (3.3V − Vf) ÷ R**

| LED | Vf (대략) | 220Ω 일 때 전류 | 100Ω 일 때 | 10kΩ 일 때 |
|---|---|---|---|---|
| 빨강 | 2.0V | 약 5.9mA | 13mA | 0.13mA (거의 안 보임) |
| 노랑 | 2.1V | 약 5.5mA | 12mA | 0.12mA |
| 초록 | 2.2V | 약 5.0mA | 11mA | 0.11mA |

키트에는 220Ω, 10kΩ, 100Ω 이 있습니다. **LED 는 220Ω** 이 기준입니다. 색띠: 220Ω = 빨강 빨강 갈색, 10kΩ = 갈색 검정 주황, 100Ω = 갈색 검정 갈색.

### ▶ 예제 3-2 · LED 3개를 배열로 제어

<img src="img/ex3_2.svg" alt="ex3_2 배선">

LED 한 개의 원리는 **준비의 그림 2** 를 봅니다.

`leds.ino`
```c
const int LED_PINS[] = {25, 26, 27};           // 빨강, 노랑, 초록
const int N_LED = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

void setup() {
    for (int i = 0; i < N_LED; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }
}

void loop() {
    for (int i = 0; i < N_LED; i++) {
        digitalWrite(LED_PINS[i], HIGH);       // 3.3V 출력
        delay(300);
        digitalWrite(LED_PINS[i], LOW);        // 0V 출력
    }
}
```

`pinMode(pin, OUTPUT)` 을 빼면 핀이 출력으로 설정되지 않아 `digitalWrite` 를 해도 LED 가 켜지지 않거나 아주 희미합니다.

### ▶ 예제 3-3 · 신호등 — enum 과 switch

<img src="img/ex3_3.svg" alt="ex3_3 배선">

`traffic.ino`
```c
typedef enum { ST_RED, ST_GREEN, ST_YELLOW } light_t;

const int PIN_R = 25, PIN_Y = 26, PIN_G = 27;

void setLights(int r, int y, int g) {
    digitalWrite(PIN_R, r);
    digitalWrite(PIN_Y, y);
    digitalWrite(PIN_G, g);
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_R, OUTPUT);
    pinMode(PIN_Y, OUTPUT);
    pinMode(PIN_G, OUTPUT);
}

void loop() {
    static light_t state = ST_RED;            // static: loop 가 끝나도 값 유지
    switch (state) {
        case ST_RED:
            setLights(1, 0, 0); Serial.println("RED");
            delay(3000); state = ST_GREEN;  break;
        case ST_GREEN:
            setLights(0, 0, 1); Serial.println("GREEN");
            delay(3000); state = ST_YELLOW; break;
        case ST_YELLOW:
            setLights(0, 1, 0); Serial.println("YELLOW");
            delay(1000); state = ST_RED;    break;
    }
}
// → RED / GREEN / YELLOW / RED / ...
```

`static` 을 빼면 `loop()` 가 불릴 때마다 `state` 가 `ST_RED` 로 다시 초기화되어 **빨강만 계속** 켜집니다. C 과정의 지역 변수 수명 그대로입니다.

### ▶ 예제 3-4 · ⚠ 함정 — LED 가 안 켜질 때 점검 순서

| 순서 | 확인 | 흔한 원인 |
|---|---|---|
| 1 | LED 방향 | 긴 다리(+)가 저항 쪽이 아님 |
| 2 | 브레드보드 열 | 가운데 홈을 기준으로 위아래가 끊겨 있음. 같은 **5칸 줄**에 꽂혔는지 |
| 3 | GND | LED(−) 가 보드의 GND 까지 이어졌는지 |
| 4 | 핀 번호 | 보드 표기 `25` 와 코드 `25` 가 같은지. 입력 전용(34~39) 이 아닌지 |
| 5 | `pinMode` | `OUTPUT` 설정을 빠뜨리지 않았는지 |

**저항 없이** LED 를 연결하면 전류가 핀 한계를 넘어 LED 나 GPIO 가 손상될 수 있습니다. 오늘은 한 번도 하지 않습니다.

#### ✏️ 빈칸 채우기 3-1

<img src="img/b3_1.svg" alt="b3_1 배선">

신호등에 `ST_BLINK` 상태를 추가합니다. 노랑 이후 노란불이 3번 깜빡인 뒤 빨강으로 갑니다.

```c
typedef enum { ST_RED, ST_GREEN, ST_YELLOW, ____ } light_t;

void loop() {
    static light_t state = ST_RED;
    switch (state) {
        case ST_RED:    setLights(1, 0, 0); delay(3000); state = ST_GREEN;  break;
        case ST_GREEN:  setLights(0, 0, 1); delay(3000); state = ST_YELLOW; break;
        case ST_YELLOW: setLights(0, 1, 0); delay(1000); state = ST_BLINK;  break;
        case ST_BLINK:
            for (int i = 0; i < ____; i++) {
                setLights(0, 1, 0); delay(250);
                setLights(0, 0, ____); delay(250);
            }
            state = ____;
            break;
    }
}
```

<details><summary>🔒 정답 보기 3-1</summary>

```c
typedef enum { ST_RED, ST_GREEN, ST_YELLOW, ST_BLINK } light_t;
            for (int i = 0; i < 3; i++) {
                setLights(0, 1, 0); delay(250);
                setLights(0, 0, 0); delay(250);
            }
            state = ST_RED;
```

상태를 하나 추가할 때 `enum` 에 이름을 넣고 `case` 하나를 더 쓰면 됩니다. 8교시에는 이 `switch` 를 **함수 포인터 표**로 바꿉니다.
</details>

#### ✏️ 괄호 넣기 3-2

1. 빨강 LED(Vf 2.0V)에 220Ω 을 쓰면 전류는 약 (　　　)mA 이다.
2. 저항을 220Ω 에서 100Ω 으로 바꾸면 LED 는 (`밝아진다` / `어두워진다`).
3. 핀을 출력으로 쓰려면 먼저 `pinMode(pin, (　　　))` 을 호출한다.
4. `loop()` 안의 지역 변수가 다음 호출까지 값을 유지하려면 (　　　) 키워드를 붙인다.
5. `digitalWrite(pin, HIGH)` 는 핀에 약 (　　　)V 를 출력한다.

<details><summary>🔒 정답 보기 3-2</summary>

1. 5.9 (약 6)
2. 밝아진다
3. OUTPUT
4. static
5. 3.3
</details>

---

# 4교시 · 레지스터 직접 제어 (13:00–13:50)

**학습 목표**
- GPIO 가 메모리 주소에 있는 레지스터로 제어된다는 것을 안다
- `W1TS` / `W1TC` 레지스터에 비트마스크를 써서 여러 핀을 한 번에 제어한다
- `digitalWrite` 와 레지스터 방식의 속도를 측정해 비교한다

🎬 **D1-4 W1TS / W1TC 비트 뷰어** — 마스크를 만들어 쓰고 OUT 레지스터 변화 관찰

### ▶ 예제 4-1 · 비트 연산 복습 — C 과정 Day 3 회수

`bits.ino`
```c
void print_bits(uint32_t v) {
    for (int i = 31; i >= 0; i--) {
        Serial.print((v >> i) & 1);
        if (i % 8 == 0 && i) Serial.print('_');
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(500);
    uint32_t reg = 0;
    reg |= (1UL << 25);              // 25번 비트 켜기
    reg |= (1UL << 27);              // 27번 비트 켜기
    print_bits(reg);                 // → 00001010_00000000_00000000_00000000
    reg &= ~(1UL << 25);             // 25번 비트 끄기
    print_bits(reg);                 // → 00001000_00000000_00000000_00000000
    Serial.println((reg >> 27) & 1); // → 1  (27번 비트 검사)
}

void loop() {}
```

`1UL` 의 `UL` 을 빼고 `1 << 31` 처럼 쓰면 `int` 의 부호 비트를 건드려 경고가 납니다. 레지스터 마스크에는 **항상 `1UL`**.

### ▶ 예제 4-2 · 레지스터로 LED 3개를 한 번에

<img src="img/ex4_2.svg" alt="ex4_2 배선">

ESP32 의 GPIO 출력은 메모리 주소에 있는 32비트 레지스터로 제어됩니다.

| 레지스터 | 쓰기 동작 |
|---|---|
| `GPIO_OUT_REG` | 32비트 전체를 덮어씀 (0~31번 핀의 출력값) |
| `GPIO_OUT_W1TS_REG` | 1 인 비트만 **켜기** (Write 1 To Set) |
| `GPIO_OUT_W1TC_REG` | 1 인 비트만 **끄기** (Write 1 To Clear) |

`reg_leds.ino`
```c
#include "soc/soc.h"
#include "soc/gpio_reg.h"

#define LED_MASK ((1UL << 25) | (1UL << 26) | (1UL << 27))

void setup() {
    pinMode(25, OUTPUT);             // 방향 설정은 그대로 필요
    pinMode(26, OUTPUT);
    pinMode(27, OUTPUT);
}

void loop() {
    REG_WRITE(GPIO_OUT_W1TS_REG, LED_MASK);   // 3개 동시에 ON
    delay(500);
    REG_WRITE(GPIO_OUT_W1TC_REG, LED_MASK);   // 3개 동시에 OFF
    delay(500);
}
```

`W1TS` 에 쓰면 **0 인 비트는 아무 영향이 없습니다.** 그래서 다른 핀의 상태를 읽어 올 필요가 없고(읽기-수정-쓰기 불필요), 인터럽트가 끼어들어도 다른 핀을 망가뜨리지 않습니다.

### ▶ 예제 4-3 · 몇 배 빠른가 — micros() 로 측정

<img src="img/ex4_3.svg" alt="ex4_3 배선">

`reg_speed.ino`
```c
#include "soc/soc.h"
#include "soc/gpio_reg.h"

const int PIN = 25;
const long N = 100000;

void setup() {
    Serial.begin(115200);
    delay(500);
    pinMode(PIN, OUTPUT);

    uint32_t t0 = micros();
    for (long i = 0; i < N; i++) {
        digitalWrite(PIN, HIGH);
        digitalWrite(PIN, LOW);
    }
    uint32_t t_api = micros() - t0;

    t0 = micros();
    for (long i = 0; i < N; i++) {
        REG_WRITE(GPIO_OUT_W1TS_REG, 1UL << PIN);
        REG_WRITE(GPIO_OUT_W1TC_REG, 1UL << PIN);
    }
    uint32_t t_reg = micros() - t0;

    Serial.printf("digitalWrite : %lu us\n", t_api);
    Serial.printf("register     : %lu us\n", t_reg);
    Serial.printf("ratio        : %.1f x\n", (float)t_api / t_reg);
}

void loop() {}
```

결과는 core 버전과 보드마다 다르니 **자기 숫자를 표에 적습니다.** `digitalWrite` 는 핀 번호 검사와 핀 관리 테이블 조회를 거치고, 레지스터 방식은 주소 하나에 값을 쓰는 명령 하나입니다. 대부분의 경우 `digitalWrite` 로 충분하고, **빠른 신호를 직접 만들어야 할 때**(Day 3 의 74HC595 등)만 레지스터를 씁니다.

### ▶ 예제 4-4 · ⚠ 함정 — 레지스터를 잘못 쓰면 남의 핀이 꺼진다

```c
REG_WRITE(GPIO_OUT_REG, 1UL << 25);     // 25번만 켜려는 의도
```

`GPIO_OUT_REG` 는 32비트를 **통째로 덮어씁니다.** 26, 27번을 포함해 0~31번 핀의 출력이 전부 0 이 됩니다. 한 핀만 바꿀 때는 **W1TS / W1TC** 를 씁니다.

| 실수 | 결과 |
|---|---|
| `GPIO_OUT_REG` 에 한 비트만 씀 | 다른 핀 전부 LOW |
| `pinMode` 없이 레지스터만 씀 | 출력이 활성화되지 않아 LED 가 안 켜짐 |
| GPIO 32, 33 을 `1UL << 32` 로 제어 | 32비트를 넘어감 — 32~39번은 `GPIO_OUT1_W1TS_REG` 에서 `1UL << (pin - 32)` |

#### ✏️ 빈칸 채우기 4-1

<img src="img/b4_1.svg" alt="b4_1 배선">

0~7 을 세면서 3개 LED 로 **2진수**를 표시합니다. bit0 → 25번, bit1 → 26번, bit2 → 27번.

```c
#include "soc/soc.h"
#include "soc/gpio_reg.h"

const int PINS[3] = {25, 26, 27};

void show_binary(int n) {
    uint32_t on = 0, off = 0;
    for (int b = 0; b < 3; b++) {
        if ((n >> b) & 1) on  |= (1UL << PINS[b]);
        else              ____ |= (1UL << PINS[b]);
    }
    REG_WRITE(____, on);
    REG_WRITE(GPIO_OUT_W1TC_REG, ____);
}

void setup() {
    for (int b = 0; b < 3; b++) pinMode(PINS[b], OUTPUT);
}

void loop() {
    for (int n = 0; n < 8; n++) {
        show_binary(n);
        delay(700);
    }
}
```

<details><summary>🔒 정답 보기 4-1</summary>

```c
        else              off |= (1UL << PINS[b]);
    REG_WRITE(GPIO_OUT_W1TS_REG, on);
    REG_WRITE(GPIO_OUT_W1TC_REG, off);
```

켤 비트와 끌 비트를 **마스크 두 개**로 모아 한 번씩 씁니다. 5 (= 0b101) 이면 25번과 27번이 켜집니다.
</details>

#### ✏️ 괄호 넣기 4-2

1. `REG_WRITE(GPIO_OUT_W1TS_REG, 1UL << 26)` 은 26번 핀을 (`켠다` / `끈다`).
2. W1TS 레지스터에 쓴 값에서 0 인 비트에 해당하는 핀은 (　　　).
3. 특정 비트를 끄는 C 식은 `reg &= (　　　)(1UL << n);` 이다.
4. `GPIO_OUT_REG` 에 값을 쓰면 0~31번 핀 출력을 (　　　) 덮어쓴다.
5. 32번 이상의 핀은 `GPIO_OUT1_...` 레지스터에서 `1UL << (pin − (　　　))` 로 제어한다.

<details><summary>🔒 정답 보기 4-2</summary>

1. 켠다
2. 변하지 않는다 (그대로 유지)
3. ~
4. 전부 (통째로)
5. 32
</details>

---

# 5교시 · Digital Input — 풀업과 디바운스 (14:00–14:50)

**학습 목표**
- 플로팅 입력이 왜 문제인지 알고 풀업으로 해결한다
- 버튼 채터링을 관찰하고 `millis()` 기반 논블로킹 디바운스를 구현한다
- 버튼 상태를 구조체로 관리해 여러 입력에 같은 함수를 재사용한다

🎬 **D1-5 채터링과 디바운스** — 파형에서 원시 엣지 수와 디바운스 후 눌림 수 비교

### ▶ 예제 5-1 · 플로팅 — 아무것도 안 눌렀는데 값이 바뀐다

<img src="img/ex5_1.svg" alt="ex5_1 배선">

버튼은 대각선 두 다리를 씁니다 (**준비의 그림 3**). 이 예제에서는 `INPUT` 모드라서 버튼이 GND 에만 연결되고 3.3V 쪽이 비어 있습니다.

`floating.ino`
```c
const int PIN_BTN = 4;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BTN, INPUT);          // 풀업 없음
}

void loop() {
    Serial.println(digitalRead(PIN_BTN));
    delay(100);
}
// → 0 1 1 0 1 ...  (손을 가까이 대면 더 요동)
```

버튼이 떨어져 있으면 핀이 **어디에도 연결되지 않은 상태(플로팅)** 라 주변 잡음을 그대로 읽습니다. `INPUT` 을 `INPUT_PULLUP` 으로 바꾸면 내부 저항이 핀을 3.3V 쪽으로 당겨 안 누를 때 `1`, 누를 때 `0` 으로 안정됩니다.

| 모드 | 안 누름 | 누름 | 배선 |
|---|---|---|---|
| `INPUT` | 불안정 | 불안정 | 외부 저항 필요 |
| `INPUT_PULLUP` | 1 (HIGH) | **0 (LOW)** | 핀 ↔ 버튼 ↔ GND |
| `INPUT_PULLDOWN` | 0 | 1 | 핀 ↔ 버튼 ↔ 3V3 |

### ▶ 예제 5-2 · 채터링 — 한 번 눌렀는데 여러 번

<img src="img/ex5_2.svg" alt="ex5_2 배선">

`chatter.ino`
```c
const int PIN_BTN = 4;
int last = HIGH;
int edges = 0;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BTN, INPUT_PULLUP);
}

void loop() {
    int now = digitalRead(PIN_BTN);
    if (last == HIGH && now == LOW) {        // 눌리는 순간(하강 엣지)
        edges++;
        Serial.printf("press #%d\n", edges);
    }
    last = now;
}
// 한 번 눌렀는데 → press #1 / press #2 / press #3 가 찍히는 경우가 있음
```

기계식 접점은 붙는 순간 수 ms 동안 **붙었다 떨어졌다**를 반복합니다(채터링, bounce). `loop()` 가 매우 빠르게 돌기 때문에 이 떨림을 여러 번의 누름으로 셉니다.

### ▶ 예제 5-3 · 논블로킹 디바운스 — 구조체 포인터로

<img src="img/ex5_3.svg" alt="ex5_3 배선">

`debounce.ino`
```c
typedef struct {
    int pin;
    int stable;            // 확정된 상태
    int last_raw;          // 직전에 읽은 원시 값
    uint32_t changed_at;   // 원시 값이 마지막으로 바뀐 시각
} button_t;

// 눌림(HIGH → LOW)이 확정되는 순간에만 1 을 반환
int button_pressed(button_t *b) {
    int raw = digitalRead(b->pin);
    if (raw != b->last_raw) {                 // 값이 흔들리면 타이머 재시작
        b->last_raw = raw;
        b->changed_at = millis();
    }
    if (millis() - b->changed_at > 30 && raw != b->stable) {
        b->stable = raw;                      // 30ms 동안 그대로면 확정
        return (raw == LOW);
    }
    return 0;
}

button_t btn = {4, HIGH, HIGH, 0};
const int PIN_LED = 25;
int led = 0;

void setup() {
    Serial.begin(115200);
    pinMode(btn.pin, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);
}

void loop() {
    if (button_pressed(&btn)) {
        led = !led;
        digitalWrite(PIN_LED, led);
        Serial.printf("LED %s\n", led ? "ON" : "OFF");
    }
}
// → 한 번 누를 때마다 정확히 한 줄: LED ON / LED OFF / ...
```

`delay(30)` 으로 기다리는 대신 **시각을 기록하고 비교**합니다. `loop()` 가 멈추지 않으니 다른 일(다른 버튼, LED 깜빡임)을 동시에 할 수 있습니다.

`millis() - b->changed_at` 처럼 **뺄셈으로 비교**하면 약 49.7일 뒤 `millis()` 가 0 으로 돌아와도 `unsigned` 산술 덕분에 올바르게 동작합니다. `millis() > b->changed_at + 30` 처럼 쓰면 그 순간 틀립니다.

### ▶ 예제 5-4 · 같은 함수로 기울기 센서까지

<img src="img/ex5_4.svg" alt="ex5_4 배선">

```c
button_t tilt = {17, HIGH, HIGH, 0};

void setup() {
    Serial.begin(115200);
    pinMode(btn.pin, INPUT_PULLUP);
    pinMode(tilt.pin, INPUT_PULLUP);
    pinMode(25, OUTPUT);
    pinMode(27, OUTPUT);
}

void loop() {
    if (button_pressed(&btn))  Serial.println("button");
    if (button_pressed(&tilt)) Serial.println("tilted");
    digitalWrite(27, tilt.stable == LOW);     // 기울어져 있는 동안 초록 LED
}
```

`button_t` 변수만 하나 더 만들었습니다. 상태를 **구조체에 담고 포인터로 넘기는** 이유가 이것입니다.

### ▶ 예제 5-5 · ⚠ 함정 — 34~39번에 버튼

버튼을 GPIO 34 에 연결하고 `pinMode(34, INPUT_PULLUP)` 을 해도 값이 요동칩니다. 34~39번은 **내부 풀업·풀다운이 없는** 입력 전용 핀이라 `INPUT_PULLUP` 이 무시됩니다. 쓰려면 외부 10kΩ 을 3V3 에 연결합니다. 오늘은 일반 GPIO(4, 17)를 씁니다.

#### ✏️ 빈칸 채우기 5-1

<img src="img/b5_1.svg" alt="b5_1 배선">

디바운스 시간을 구조체 멤버로 옮겨 입력마다 다르게 설정합니다 (버튼 30ms, 기울기 센서 100ms).

```c
typedef struct {
    int pin;
    int stable;
    int last_raw;
    uint32_t changed_at;
    uint32_t ____;
} button_t;

int button_pressed(button_t *b) {
    int raw = digitalRead(b->pin);
    if (raw != b->last_raw) {
        b->last_raw = raw;
        b->changed_at = ____;
    }
    if (millis() - b->changed_at > b->debounce_ms && raw != ____) {
        b->stable = raw;
        return (raw == LOW);
    }
    return 0;
}

button_t btn  = {4,  HIGH, HIGH, 0, 30};
button_t tilt = {17, HIGH, HIGH, 0, ____};
```

<details><summary>🔒 정답 보기 5-1</summary>

```c
    uint32_t debounce_ms;
        b->changed_at = millis();
    if (millis() - b->changed_at > b->debounce_ms && raw != b->stable) {
button_t tilt = {17, HIGH, HIGH, 0, 100};
```

기울기 센서의 금속 공은 버튼보다 오래 흔들리므로 더 긴 시간을 줍니다. **설정값도 구조체에** — Day 2 센서 구조체 설계의 예고입니다.
</details>

#### ✏️ 괄호 넣기 5-2

1. 어디에도 연결되지 않아 값이 요동치는 입력 상태를 (　　　)이라 한다.
2. `INPUT_PULLUP` 모드에서 버튼을 누르면 `digitalRead` 는 (`HIGH` / `LOW`) 를 반환한다.
3. 기계식 접점이 붙는 순간 여러 번 붙었다 떨어지는 현상을 (　　　)이라 한다.
4. `delay` 없이 시각을 기록해 비교하는 방식을 (　　　) 디바운스라 한다.
5. GPIO 34~39 는 내부 (　　　) 저항이 없어 `INPUT_PULLUP` 이 동작하지 않는다.

<details><summary>🔒 정답 보기 5-2</summary>

1. 플로팅 (floating)
2. LOW
3. 채터링 (bounce)
4. 논블로킹
5. 풀업 (풀다운 포함)
</details>

---

# 6교시 · Interrupt — ISR 과 공유 변수 (15:00–15:50)

**학습 목표**
- 폴링의 한계를 확인하고 GPIO 인터럽트를 등록한다
- ISR 작성 규칙(`IRAM_ATTR`, 짧게, `delay`·`Serial` 금지)을 지킨다
- `volatile` 과 임계 구역으로 ISR 과 `loop()` 가 공유하는 변수를 보호한다

🎬 **D1-6 인터럽트 타임라인** — loop 실행 중 버튼 → ISR 끼어들기, volatile · 임계 구역 켜고 끄기

### ▶ 예제 6-1 · 폴링의 한계 — loop 가 바쁘면 놓친다

<img src="img/ex6_1.svg" alt="ex6_1 배선">

`polling_miss.ino`
```c
const int PIN_BTN = 4;
int count = 0;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BTN, INPUT_PULLUP);
}

void loop() {
    if (digitalRead(PIN_BTN) == LOW) count++;
    Serial.printf("count = %d\n", count);
    delay(2000);                    // 다른 일을 하느라 바쁜 상황
}
// 2초 사이에 버튼을 빠르게 5번 눌러도 → count 는 0 또는 1 만 증가
```

`loop()` 가 `digitalRead` 를 하는 **그 순간**에 눌려 있어야만 셉니다. 이것이 폴링(polling)입니다.

### ▶ 예제 6-2 · 인터럽트 — 하드웨어가 알려 준다

<img src="img/ex6_2.svg" alt="ex6_2 배선">

`isr_counter.ino`
```c
const int PIN_BTN = 4;

volatile uint32_t press_count = 0;
volatile uint32_t last_isr_ms = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onButton() {
    uint32_t now = millis();
    if (now - last_isr_ms > 50) {             // ISR 안의 간단한 디바운스
        portENTER_CRITICAL_ISR(&mux);
        press_count++;
        portEXIT_CRITICAL_ISR(&mux);
        last_isr_ms = now;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BTN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN), onButton, FALLING);
}

void loop() {
    portENTER_CRITICAL(&mux);
    uint32_t n = press_count;                 // 읽고
    press_count = 0;                          // 비우기를 한 덩어리로
    portEXIT_CRITICAL(&mux);

    if (n) Serial.printf("pressed %lu time(s)\n", n);
    delay(2000);                              // 여전히 바빠도
}
// 2초 사이에 5번 누르면 → pressed 5 time(s)
```

| 요소 | 의미 |
|---|---|
| `IRAM_ATTR` | ISR 코드를 Flash 가 아닌 **내부 RAM** 에 둔다. Flash 접근 중에도 즉시 실행 가능 |
| `FALLING` | HIGH → LOW 로 떨어지는 순간 (풀업 버튼의 눌림). `RISING`, `CHANGE` 도 있음 |
| `volatile` | 컴파일러에게 "이 값은 코드 밖에서 바뀐다, 매번 메모리에서 읽어라" |
| `portENTER_CRITICAL` | 이 구간 동안 인터럽트가 끼어들지 못하게 함 |

`attachInterrupt` 의 두 번째 인자 `onButton` 은 **함수 포인터**입니다. C 과정 Day 3 의 `qsort(arr, n, size, cmp)` 에 비교 함수를 넘기던 것과 같은 모양입니다.

### ▶ 예제 6-3 · ⚠ 함정 — ISR 안에서 하면 안 되는 것

```c
void IRAM_ATTR onButton() {
    Serial.println("pressed!");     // ✗ 내부 잠금(lock)을 기다리다 멈추거나 크래시
    delay(100);                     // ✗ ISR 안에서 시간이 흐르지 않음
    count++;                        // count 가 volatile 이 아니면
}                                   //   loop 가 바뀐 값을 못 볼 수 있음
```

규칙은 하나입니다. **ISR 은 표시만 하고 나온다.** 카운트를 올리거나 플래그를 세우고, 출력·대기·계산은 `loop()` 에서 합니다.

**임계 구역을 빼면?** `loop()` 가 `n = press_count;` 를 실행한 직후, `press_count = 0;` 직전에 ISR 이 들어와 1을 더하면, 그 1은 곧바로 0 으로 지워져 **눌림 하나가 사라집니다.** 드물게 일어나지만 반드시 일어나는 버그입니다 (🎬 D1-6 에서 재현).

#### ✏️ 빈칸 채우기 6-1

<img src="img/b6_1.svg" alt="b6_1 배선">

버튼을 누를 때마다 인터럽트로 LED 를 토글합니다. ISR 은 플래그만 세웁니다.

```c
const int PIN_BTN = 4, PIN_LED = 25;
____ bool flag = false;
volatile uint32_t last_ms = 0;

void ____ onButton() {
    uint32_t now = millis();
    if (now - last_ms > 50) {
        flag = true;
        last_ms = now;
    }
}

void setup() {
    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN), onButton, ____);
}

void loop() {
    if (flag) {
        flag = false;
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    }
}
```

<details><summary>🔒 정답 보기 6-1</summary>

```c
volatile bool flag = false;
void IRAM_ATTR onButton() {
    attachInterrupt(digitalPinToInterrupt(PIN_BTN), onButton, FALLING);
```

`bool` 한 개를 쓰고 지우는 것은 한 번의 메모리 접근이라 이 예제에서는 임계 구역 없이도 안전합니다. **읽고-바꾸고-쓰는** 동작(`count++`, 읽고 0 으로 만들기)이 있을 때 임계 구역이 필요합니다.
</details>

#### ✏️ 괄호 넣기 6-2

1. 주기적으로 핀을 읽어 변화를 확인하는 방식을 (　　　), 하드웨어가 변화를 알려 주는 방식을 (　　　)라 한다.
2. ESP32 에서 ISR 함수 앞에는 (　　　) 를 붙여 내부 RAM 에 둔다.
3. ISR 과 `loop()` 가 공유하는 변수에는 (　　　) 를 붙인다.
4. 풀업 버튼의 눌림을 감지하려면 인터럽트 모드를 (`RISING` / `FALLING`) 으로 한다.
5. 공유 변수를 읽고 0 으로 만드는 두 동작 사이에 ISR 이 끼어들지 못하게 하는 구간을 (　　　)이라 한다.

<details><summary>🔒 정답 보기 6-2</summary>

1. 폴링 / 인터럽트
2. IRAM_ATTR
3. volatile
4. FALLING
5. 임계 구역 (critical section)
</details>

---

# 7교시 · PWM (LEDC) — 밝기 · 색 · 소리 (16:00–16:50)

**학습 목표**
- 주파수, 듀티비, 분해능으로 PWM 신호를 설명한다
- core 3.x 의 `ledcAttach` / `ledcWrite` 로 LED 밝기와 RGB 색을 제어한다
- `ledcWriteTone` 과 구조체 배열로 멜로디를 재생한다

🎬 **D1-7 듀티비 시각화** — 듀티·분해능·주파수를 바꿔 파형과 밝기 비교

### ▶ 예제 7-1 · 디지털 핀으로 밝기를 — 빠르게 켜고 끄기

<img src="img/ex7_1.svg" alt="ex7_1 배선">

PWM(Pulse Width Modulation)은 핀을 **아주 빠르게 켰다 껐다** 하면서 켜진 시간의 비율(듀티비)을 바꾸는 방식입니다. 5000Hz 로 깜빡이면 눈은 평균 밝기만 봅니다.

| 용어 | 의미 | 예 |
|---|---|---|
| 주파수 | 1초에 켜고 끄는 횟수 | 5000 Hz |
| 분해능 | 듀티를 몇 단계로 나눌지 (비트 수) | 8비트 → 0~255 |
| 듀티 | 켜진 시간의 비율 | 128 / 255 ≈ 50% |

`fade.ino`
```c
const int PIN_LED = 25;

void setup() {
    ledcAttach(PIN_LED, 5000, 8);        // 핀, 주파수 5kHz, 8비트(0~255)
}

void loop() {
    for (int d = 0; d <= 255; d++) { ledcWrite(PIN_LED, d); delay(5); }
    for (int d = 255; d >= 0; d--) { ledcWrite(PIN_LED, d); delay(5); }
}
```

### ▶ 예제 7-2 · RGB 색 혼합 — 구조체 배열

<img src="img/wiring_rgb.svg" alt="그림 4 RGB LED 배선">

RGB LED 는 비어 있는 **40~43열**에 꽂습니다. 저항 3개를 `e40–f40 · e42–f42 · e43–f43` 에 추가하고, LED 3개에 꽂혀 있던 **GPIO 25 · 26 · 27 선만 뽑아** `a40 · a42 · a43` 으로 옮깁니다. 다른 부품은 그대로 두어도 됩니다. 공통 단자 열(41)은 `j41` 에서 − 레일(캐소드형) 또는 + 레일(애노드형, + 레일을 3V3 에 연결)로 잇습니다.

`rgb.ino`
```c
typedef struct {
    uint8_t r, g, b;
    const char *name;
} color_t;

const color_t COLORS[] = {
    {255,   0,   0, "red"},
    {  0, 255,   0, "green"},
    {  0,   0, 255, "blue"},
    {255, 180,   0, "orange"},
    {160,   0, 255, "purple"},
    {255, 255, 255, "white"},
};
const int N_COLOR = sizeof(COLORS) / sizeof(COLORS[0]);

const int PIN_R = 25, PIN_G = 26, PIN_B = 27;
#define COMMON_ANODE 0                 // 공통 애노드면 1 로

void set_rgb(const color_t *c) {
    uint8_t r = c->r, g = c->g, b = c->b;
#if COMMON_ANODE
    r = 255 - r; g = 255 - g; b = 255 - b;   // 애노드형은 LOW 가 켜짐
#endif
    ledcWrite(PIN_R, r);
    ledcWrite(PIN_G, g);
    ledcWrite(PIN_B, b);
}

void setup() {
    Serial.begin(115200);
    ledcAttach(PIN_R, 5000, 8);
    ledcAttach(PIN_G, 5000, 8);
    ledcAttach(PIN_B, 5000, 8);
}

void loop() {
    for (int i = 0; i < N_COLOR; i++) {
        set_rgb(&COLORS[i]);
        Serial.println(COLORS[i].name);
        delay(800);
    }
}
```

**공통 캐소드인지 애노드인지 먼저 확인합니다.** 가장 긴 다리를 GND 에 꽂고 빨강이 나오면 캐소드형, 3V3 에 꽂아야 나오면 애노드형입니다. 애노드형은 `COMMON_ANODE` 를 1 로 바꿉니다 — 전처리기 `#if` 는 C 과정 Day 1 의 조건부 컴파일입니다.

### ▶ 예제 7-3 · 부저 두 종류

<img src="img/ex7_3.svg" alt="ex7_3 배선">

| 종류 | 구분 | 소리 내는 법 |
|---|---|---|
| Active Buzzer | 뒷면이 막혀 있음(검은 수지), 보통 스티커 | `digitalWrite(19, HIGH)` — 내장 발진기로 정해진 음 |
| Passive Buzzer | 뒷면에 기판이 보임 | `ledcWriteTone(23, 440)` — 주파수를 우리가 만듦 |

`buzzers.ino`
```c
void setup() {
    pinMode(19, OUTPUT);
    ledcAttach(23, 1000, 10);
}

void loop() {
    digitalWrite(19, HIGH); delay(200); digitalWrite(19, LOW);   // 삐
    delay(500);
    ledcWriteTone(23, 440); delay(200); ledcWriteTone(23, 0);    // 라(A4)
    delay(1500);
}
```

### ▶ 예제 7-4 · 멜로디 — {주파수, 길이} 구조체 배열

<img src="img/ex7_4.svg" alt="ex7_4 배선">

`melody.ino`
```c
typedef struct { uint16_t freq; uint16_t ms; } note_t;

const note_t melody[] = {
    {262, 300}, {294, 300}, {330, 300}, {349, 300},   // 도 레 미 파
    {392, 300}, {440, 300}, {494, 300}, {523, 600},   // 솔 라 시 도
};
const int N = sizeof(melody) / sizeof(melody[0]);
const int PIN_BUZ = 23;

void setup() {
    ledcAttach(PIN_BUZ, 1000, 10);
}

void loop() {
    for (int i = 0; i < N; i++) {
        ledcWriteTone(PIN_BUZ, melody[i].freq);
        delay(melody[i].ms);
    }
    ledcWriteTone(PIN_BUZ, 0);            // 0 Hz = 소리 끄기
    delay(2000);
}
```

악보가 **데이터(배열)** 이고 연주가 **코드(루프)** 입니다. 곡을 바꿀 때 코드를 고칠 필요가 없습니다. 쉼표는 `{0, 300}` 으로 넣습니다.

### ▶ 예제 7-5 · ⚠ 함정 — 인터넷 예제가 컴파일되지 않는다

```c
ledcSetup(0, 5000, 8);       // core 2.x 방식
ledcAttachPin(25, 0);
ledcWrite(0, 128);           // 채널 번호로 쓰기
```

```
error: 'ledcSetup' was not declared in this scope
```

core 3.x 에서 LEDC API 가 **채널 중심 → 핀 중심**으로 바뀌었습니다. 검색으로 찾은 예제가 2.x 방식이면 `ledcAttach(pin, freq, bits)` + `ledcWrite(pin, duty)` 로 바꿉니다. 과정 내내 core 버전을 고정하는 이유입니다.

#### ✏️ 빈칸 채우기 7-1

<img src="img/b7_1.svg" alt="b7_1 배선">

생일 축하 노래 첫 소절입니다. 쉼표(0 Hz)가 들어 있습니다.

```c
typedef struct { uint16_t freq; uint16_t ms; } note_t;

const note_t song[] = {
    {262, 250}, {262, 250}, {294, 500}, {262, 500},
    {349, 500}, {330, 900}, {____, 300},             // 쉼표
};
const int N = sizeof(song) / ____;
const int PIN_BUZ = 23;

void setup() {
    ledcAttach(PIN_BUZ, ____, 10);
}

void loop() {
    for (int i = 0; i < N; i++) {
        ____(PIN_BUZ, song[i].freq);
        delay(song[i].ms);
    }
    ledcWriteTone(PIN_BUZ, 0);
    delay(2000);
}
```

<details><summary>🔒 정답 보기 7-1</summary>

```c
    {349, 500}, {330, 900}, {0, 300},
const int N = sizeof(song) / sizeof(song[0]);
    ledcAttach(PIN_BUZ, 1000, 10);
        ledcWriteTone(PIN_BUZ, song[i].freq);
```

`ledcAttach` 의 주파수는 시작값일 뿐이라 1000 이 아니어도 동작합니다. `ledcWriteTone` 이 호출될 때마다 주파수를 새로 설정합니다.
</details>

#### ✏️ 괄호 넣기 7-2

1. PWM 에서 한 주기 중 HIGH 인 시간의 비율을 (　　　)라 한다.
2. 분해능이 8비트이면 듀티 값의 범위는 0 ~ (　　　) 이다.
3. core 3.x 에서 핀에 PWM 을 연결하는 함수는 (　　　)(pin, freq, bits) 이다.
4. 주파수를 바꿔 음을 낼 수 있는 부저는 (`Active` / `Passive`) Buzzer 이다.
5. 공통 애노드 RGB LED 는 듀티를 (　　　) 해야 원하는 색이 나온다.

<details><summary>🔒 정답 보기 7-2</summary>

1. 듀티비 (duty cycle)
2. 255
3. ledcAttach
4. Passive
5. 반전 (255 − 값)
</details>

---

# 8교시 · 미니 프로젝트 — 반응속도 게임 · 정리 (17:00–17:50)

**학습 목표**
- 함수 포인터 표로 상태 머신을 구성한다
- Day 1 의 입력(디바운스), 출력(LED), PWM(부저)을 하나의 프로그램으로 통합한다

🎬 **D1-8 함수 포인터 상태 머신** — `table[current]()` 한 줄이 상태를 옮기는 과정 재생

### 게임 규칙

1. **IDLE** — 버튼을 누르면 시작. 1~4초 사이 랜덤 시간을 정한다.
2. **WAIT** — LED 꺼진 채 기다린다. 이때 누르면 **반칙(FOUL)**.
3. **GO** — LED 가 켜진다. 누를 때까지의 시간(ms)을 잰다. 3초 동안 안 누르면 시간 초과.
4. **RESULT** — 반응 시간을 출력하고, 최고 기록이면 알린다. 300ms 미만이면 좋은 멜로디, 아니면 낮은 멜로디.
5. **FOUL** — 반칙 메시지, 낮은 멜로디, LED 3번 깜빡임.

```
        누름                 시간 경과              누름
 IDLE ───────▶ WAIT ─────────────────▶ GO ─────────────▶ RESULT
   ▲             │ 누름(반칙)            │ 3초 초과          │
   │             ▼                      ▼                   │
   └──────────  FOUL ◀── (끝나면 IDLE) ──┴────── IDLE ◀──────┘
```

### ▶ 예제 8-1 · switch 대신 함수 포인터 표

```c
typedef enum { ST_IDLE, ST_WAIT, ST_GO, ST_RESULT, ST_FOUL, ST_COUNT } state_t;
typedef state_t (*state_fn)(void);     // "state_t 를 돌려주는 인자 없는 함수"의 포인터

state_t on_idle(void);
state_t on_wait(void);
state_t on_go(void);
state_t on_result(void);
state_t on_foul(void);

const state_fn table[ST_COUNT] = { on_idle, on_wait, on_go, on_result, on_foul };
state_t current = ST_IDLE;

void loop() {
    current = table[current]();        // 현재 상태 함수를 실행하고, 다음 상태를 받는다
}
```

`enum` 값이 0, 1, 2… 이므로 그대로 배열 인덱스가 됩니다. 상태가 늘어도 `loop()` 는 **한 줄 그대로**이고, 각 상태의 코드는 자기 함수 안에만 있습니다. `ST_COUNT` 는 "상태 개수"를 자동으로 세는 관용구입니다.

### ✏️ 종합 과제 8-A · 반응속도 게임 완성

<img src="img/ex8_A.svg" alt="ex8_A 배선">

아래 뼈대의 `// TODO` 를 채웁니다. 배선은 **준비의 그림 1** 그대로입니다 (LED 25, 버튼 4, Passive Buzzer 23 사용). 7교시에 RGB 로 옮긴 선은 원래 자리로 되돌립니다.

`reaction_game.ino`
```c
#define LEN(a) (sizeof(a) / sizeof((a)[0]))

const int PIN_LED = 25, PIN_BTN = 4, PIN_BUZ = 23;

/* ---------- 5교시: 디바운스 ---------- */
typedef struct {
    int pin; int stable; int last_raw; uint32_t changed_at;
} button_t;

button_t btn = {PIN_BTN, HIGH, HIGH, 0};

int button_pressed(button_t *b) {
    int raw = digitalRead(b->pin);
    if (raw != b->last_raw) { b->last_raw = raw; b->changed_at = millis(); }
    if (millis() - b->changed_at > 30 && raw != b->stable) {
        b->stable = raw;
        return (raw == LOW);
    }
    return 0;
}

/* ---------- 7교시: 멜로디 ---------- */
typedef struct { uint16_t freq; uint16_t ms; } note_t;
const note_t GOOD[] = {{523, 120}, {659, 120}, {784, 250}};
const note_t BAD[]  = {{392, 200}, {262, 400}};

void play(const note_t *m, int n) {
    for (int i = 0; i < n; i++) { ledcWriteTone(PIN_BUZ, m[i].freq); delay(m[i].ms); }
    ledcWriteTone(PIN_BUZ, 0);
}

/* ---------- 8교시: 상태 머신 ---------- */
typedef enum { ST_IDLE, ST_WAIT, ST_GO, ST_RESULT, ST_FOUL, ST_COUNT } state_t;
typedef state_t (*state_fn)(void);

uint32_t t_mark;                  // 현재 상태에 들어온 시각
uint32_t wait_ms;                 // WAIT 에서 기다릴 시간
uint32_t reaction_ms;             // 측정 결과
uint32_t best_ms = UINT32_MAX;    // 최고 기록

state_t on_idle(void) {
    if (button_pressed(&btn)) {
        wait_ms = random(1000, 4001);
        t_mark = millis();
        Serial.println("준비... 불이 켜지면 누르세요");
        return ST_WAIT;
    }
    return ST_IDLE;
}

state_t on_wait(void) {
    // TODO 1: 버튼이 눌리면 반칙 상태로
    // TODO 2: wait_ms 가 지나면 LED 켜고, t_mark 갱신 후 GO 상태로
    return ST_WAIT;
}

state_t on_go(void) {
    // TODO 3: 버튼이 눌리면 reaction_ms 계산, LED 끄고 RESULT 로
    // TODO 4: 3000ms 가 지나면 LED 끄고 "시간 초과" 출력 후 IDLE 로
    return ST_GO;
}

state_t on_result(void) {
    Serial.printf("반응 시간: %lu ms\n", reaction_ms);
    // TODO 5: 최고 기록이면 갱신하고 "신기록!" 출력
    // TODO 6: 300ms 미만이면 GOOD, 아니면 BAD 재생
    Serial.println("버튼을 눌러 다시 시작");
    return ST_IDLE;
}

state_t on_foul(void) {
    Serial.println("반칙! 불이 켜지기 전에 눌렀습니다");
    play(BAD, LEN(BAD));
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_LED, HIGH); delay(150);
        digitalWrite(PIN_LED, LOW);  delay(150);
    }
    return ST_IDLE;
}

const state_fn table[ST_COUNT] = { on_idle, on_wait, on_go, on_result, on_foul };
state_t current = ST_IDLE;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BTN, INPUT_PULLUP);
    ledcAttach(PIN_BUZ, 1000, 10);
    Serial.println("반응속도 게임 — 버튼을 눌러 시작");
}

void loop() {
    current = table[current]();
}
```

기대 출력 예:
```
반응속도 게임 — 버튼을 눌러 시작
준비... 불이 켜지면 누르세요
반응 시간: 287 ms
신기록!
버튼을 눌러 다시 시작
준비... 불이 켜지면 누르세요
반칙! 불이 켜지기 전에 눌렀습니다
```

<details><summary>🔒 정답 보기 8-A (리뷰 시간에 함께 엽니다)</summary>

```c
state_t on_wait(void) {
    if (button_pressed(&btn)) return ST_FOUL;
    if (millis() - t_mark >= wait_ms) {
        digitalWrite(PIN_LED, HIGH);
        t_mark = millis();
        return ST_GO;
    }
    return ST_WAIT;
}

state_t on_go(void) {
    if (button_pressed(&btn)) {
        reaction_ms = millis() - t_mark;
        digitalWrite(PIN_LED, LOW);
        return ST_RESULT;
    }
    if (millis() - t_mark > 3000) {
        digitalWrite(PIN_LED, LOW);
        Serial.println("시간 초과");
        return ST_IDLE;
    }
    return ST_GO;
}

state_t on_result(void) {
    Serial.printf("반응 시간: %lu ms\n", reaction_ms);
    if (reaction_ms < best_ms) {
        best_ms = reaction_ms;
        Serial.println("신기록!");
    }
    if (reaction_ms < 300) play(GOOD, LEN(GOOD));
    else                   play(BAD, LEN(BAD));
    Serial.println("버튼을 눌러 다시 시작");
    return ST_IDLE;
}
```
</details>

**확장 과제 (선택)**
- 3판을 하고 평균을 출력한다 (상태 하나 추가: `ST_SUMMARY`)
- 버튼 2개(GPIO 4, 16)로 2인 대결 — 먼저 누른 쪽 승리, `button_t` 를 배열로
- 대기 중에는 노랑 LED(26)를 PWM 으로 천천히 숨쉬게 하기 — `delay` 없이

**제출 체크리스트**
- [ ] 정상 게임에서 반응 시간이 ms 단위로 출력된다
- [ ] WAIT 중 누르면 반칙으로 처리되고 게임이 멈추지 않는다
- [ ] 한 번 누름이 한 번으로 인식된다 (디바운스)
- [ ] 상태가 함수 + 함수 포인터 표로 구성되어 있다 (`switch` 하나에 몰아넣지 않음)
- [ ] 미완성이어도 TODO 1~4 가 동작하면 통과

### 오늘의 여섯 문장

1. ESP32 는 3.3V 칩이다. 입력 전용(34~39), Strapping(0·2·5·12·15), Flash(6~11) 핀을 피한다 (D1-1)
2. `.ino` → `.cpp` → `.elf` → `.bin` → Flash 0x10000. `setup`/`loop` 는 `loopTask` 안의 함수다 (D1-2)
3. LED 에는 항상 220Ω. 상태는 `enum` 으로 이름 붙이고, 유지할 지역 변수는 `static` (D1-3)
4. GPIO 는 레지스터다. 한 핀만 바꿀 때는 `W1TS`/`W1TC` 에 `1UL << pin` (D1-4)
5. 입력은 `INPUT_PULLUP` + 논블로킹 디바운스. 상태는 구조체에 담아 포인터로 넘긴다 (D1-5)
6. ISR 은 `IRAM_ATTR`, 짧게, 표시만. 공유 변수는 `volatile`, 읽고-바꾸기는 임계 구역 (D1-6)

- 개념 워크시트: `anim/worksheet.html` 8교시까지 채점 후 결과 코드 제출
- 제출: `reaction_game.ino` + 시리얼 모니터 출력 캡처 + 워크시트 캡처
- **Day 2 예고**: 아날로그 입력(ADC) — 가변저항, 조도, 조이스틱, 온습도, 가스, 초음파 센서. 오늘의 `button_t` 처럼 **센서도 구조체**로 설계하고, 링 버퍼로 이동평균 필터를 만듭니다
