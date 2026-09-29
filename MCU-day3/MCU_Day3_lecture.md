# MCU Day 3 강의 자료 — 디스플레이 · I2C · 모터

**온디바이스 AI · MCU 실습(ESP32) 3/4**

| 교시 | 주제 | 애니메이션 (anim/index.html) |
|---|---|---|
| 1 | 7-Segment — 비트맵으로 숫자 그리기 | D3-1 세그먼트 비트맵 |
| 2 | 74HC595 시프트 레지스터 — 선 3개로 8개 | D3-2 시프트 · 래치 |
| 3 | 4자리 7-Segment — 멀티플렉싱과 타이머 | D3-3 멀티플렉싱 |
| 4 | 8×8 도트 매트릭스 — 2차원 비트맵 | D3-4 매트릭스 편집기 |
| 5 | I2C 와 1602 LCD | D3-5 I2C 버스 |
| 6 | PCF8591 — 라이브러리 없이 I2C 직접 | D3-6 제어 바이트 |
| 7 | DC 모터(L293D)와 릴레이 | D3-7 H-브리지 |
| 8 | 스테퍼 모터 · 미니 프로젝트 · 정리 | D3-8 스테퍼 시퀀스 |

### 이 자료 사용법

- **▶ 예제**는 교시 시작의 **배선도대로 꽂은 뒤** 업로드합니다. 오늘은 결과가 **눈에 보이는 출력**(숫자 · 그림 · 글자 · 회전)이라 시리얼 모니터보다 부품을 보며 확인합니다.
- **✏️ 빈칸 채우기**는 `____` 를 채워 업로드합니다. **✏️ 괄호 넣기**는 ( ) 에 들어갈 말을 먼저 생각합니다.
- **🔒 정답 보기**는 강사가 신호를 준 뒤 펼칩니다.
- 오늘의 C 는 **비트 연산**이 주인공입니다: `1 << n`, `&`, `|`, `~`, `>>`. Day 1 4교시의 레지스터 비트 연산이 표시 장치에서 그대로 쓰입니다.

---

## ⚙️ 준비 — 오늘의 부품과 규칙

| 부품 | 핵심 | 주의 |
|---|---|---|
| 7-Segment (1자리 · 4자리) | 공통 음극(CC) 또는 공통 양극(CA) | **어느 쪽인지 1교시 첫 예제로 확인** |
| 74HC595 | 직렬 → 병렬 변환 IC, 16핀 | **반달 홈을 왼쪽**에 두고 가운데 홈을 건너 꽂기 |
| 8×8 매트릭스 (1088AS 계열) | 행 8 + 열 8 = 16핀 | 핀 번호가 행 · 열 순서가 아님 → 표로 확인 |
| 1602 LCD (I2C 백팩) | 선 4개 (GND · VCC · SDA · SCL) | 주소 0x27 또는 0x3F → 스캐너로 확인 |
| PCF8591 | I2C 8비트 ADC 4ch + DAC 1ch, 주소 0x48 | 전원 3V3 |
| L293D | 모터 2개용 H-브리지 IC, 16핀 | 모터는 **절대 GPIO 에 직접** 연결하지 않는다 |
| 릴레이 모듈 | 작은 신호로 큰 스위치 | **접점에 220V 를 연결하지 않는다** |
| ULN2003 + 28BYJ-48 | 스테퍼 드라이버 보드 | 전원 5V |

전원 레일은 Day 2 와 같습니다: **위 빨강 = 3V3, 파랑 = GND, 아래 빨강 = 5V(7 · 8교시)**.

> **IC 꽂는 법** — 74HC595 · L293D 같은 DIP IC 는 **반달 홈(또는 점)이 있는 쪽이 1번 핀**입니다. 홈을 **왼쪽**으로 두면 아래 줄 왼쪽부터 1, 2, 3 … 8번, 위 줄 오른쪽부터 9 … 16번입니다(반시계 방향). 가운데 홈을 건너 양쪽 다리가 서로 연결되지 않게 꽂고, 다리가 휘지 않게 수직으로 누릅니다.

### 🛠 따라하기 · LiquidCrystal I2C 라이브러리 설치 (5교시 전)

1. **라이브러리 매니저** 를 엽니다.
2. `LiquidCrystal I2C` 를 검색합니다.
3. **LiquidCrystal I2C by Frank de Brabander** 를 설치합니다.
4. 컴파일할 때 `WARNING: library LiquidCrystal I2C claims to run on avr architecture(s)…` 가 나와도 **경고일 뿐** 업로드는 됩니다.

오늘 나머지(7-Segment · 595 · 매트릭스 · PCF8591 · 모터 · 스테퍼)는 **라이브러리 없이** 직접 구현합니다.

---

# 1교시 · 7-Segment — 비트맵으로 숫자 그리기 (09:00–09:50)

**학습 목표**
- 7-Segment 의 세그먼트 이름(a~g, dp)과 공통 음극 / 공통 양극을 구분한다
- 숫자 모양을 1바이트 비트맵 배열로 표현한다
- `(pattern >> i) & 1` 로 비트를 하나씩 꺼내 핀에 출력한다

🎬 **D3-1 세그먼트 비트맵** — 세그먼트를 눌러 켜고 끄면 비트맵 바이트가 바뀌는 것 확인, CC / CA 전환

<img src="img/d3_L1.svg" alt="1교시 배선">

```text
   ─a─
  f   b         비트:  7  6  5  4  3  2  1  0
   ─g─          이름: dp  g  f  e  d  c  b  a
  e   c
   ─d─  .dp     "0" = a b c d e f 켜짐 = 0011 1111 = 0x3F
```

> 오늘 1자리는 **COM 에 220Ω 하나**만 둡니다(배선이 간단함). 대신 켜진 세그먼트가 많을수록 전류를 나눠 써서 "8" 이 "1" 보다 조금 어둡습니다. 3교시부터는 세그먼트마다 저항을 둡니다.

### ▶ 예제 1-1 · 세그먼트 하나씩 — CC 인가 CA 인가

<img src="img/d3_ex1_1.svg" alt="예제 1-1 배선">

`seg_test.ino`
```c
const int SEG_PINS[8] = {13, 14, 27, 26, 25, 33, 32, 23};   // a b c d e f g dp
const char *NAMES[8] = {"a", "b", "c", "d", "e", "f", "g", "dp"};

void setup() {
    Serial.begin(115200);
    for (int i = 0; i < 8; i++) pinMode(SEG_PINS[i], OUTPUT);
}

void loop() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) digitalWrite(SEG_PINS[j], LOW);
        digitalWrite(SEG_PINS[i], HIGH);            // 하나만 HIGH
        Serial.printf("%s 켜짐\n", NAMES[i]);
        delay(500);
    }
}
// → 공통 음극(CC): 하나씩 순서대로 켜진다
// → 공통 양극(CA): 하나만 꺼지고 나머지가 켜진다 (또는 전부 꺼짐 — COM 을 3V3 로 옮긴다)
```

**공통 음극(CC)** 은 COM 이 GND, 세그먼트에 HIGH 를 주면 켜집니다. **공통 양극(CA)** 은 COM 이 3V3, 세그먼트에 **LOW** 를 주면 켜집니다. 제품 번호 끝이 `…AS` 면 CC, `…BS` 면 CA 인 경우가 많지만 **직접 확인이 가장 확실합니다**.

### ▶ 예제 1-2 · 비트맵 배열 — 0~9

<img src="img/d3_ex1_2.svg" alt="예제 1-2 배선">

`seg_digits.ino`
```c
const int SEG_PINS[8] = {13, 14, 27, 26, 25, 33, 32, 23};   // a b c d e f g dp
const bool COMMON_ANODE = false;                            // CA 면 true

//                         dp g f e d c b a
const uint8_t DIGITS[10] = {
    0x3F,   // 0  0 0 1 1 1 1 1 1
    0x06,   // 1  0 0 0 0 0 1 1 0
    0x5B,   // 2  0 1 0 1 1 0 1 1
    0x4F,   // 3
    0x66,   // 4
    0x6D,   // 5
    0x7D,   // 6
    0x07,   // 7
    0x7F,   // 8
    0x6F,   // 9
};

void seg_show(uint8_t pattern) {
    if (COMMON_ANODE) pattern = ~pattern;           // CA 는 뒤집는다
    for (int i = 0; i < 8; i++) {
        digitalWrite(SEG_PINS[i], (pattern >> i) & 1);   // i 번째 비트
    }
}

void setup() {
    for (int i = 0; i < 8; i++) pinMode(SEG_PINS[i], OUTPUT);
}

void loop() {
    for (int d = 0; d < 10; d++) {
        seg_show(DIGITS[d]);
        delay(700);
    }
    seg_show(0x80);                                  // 점만
    delay(700);
}
```

숫자 모양이 **코드가 아니라 데이터(표)** 입니다. `(pattern >> i) & 1` 은 "i 번째 비트만 남기기" — Day 1 4교시의 `REG_READ(...) & (1UL << pin)` 과 같은 비트 꺼내기입니다.

### ▶ 예제 1-3 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| CA 부품인데 COM 을 GND 에 | 아무것도 안 켜짐 → COM 을 3V3 로, `COMMON_ANODE = true` |
| 비트 순서를 a 가 bit7 로 착각 | 엉뚱한 모양 — 오늘 규칙은 **a = bit0, dp = bit7** |
| COM 두 핀 중 하나만 연결 | 대부분 제품은 속에서 이어져 있어 동작하지만, 둘 다 연결하는 것이 원칙 |
| 저항 없이 COM 을 GND 에 직결 | 세그먼트마다 과전류 — 부품 · 핀 손상 위험 |

#### ✏️ 빈칸 채우기 1-1

16진수 한 자리(0~F)를 표시합니다. A=0x77, b=0x7C, C=0x39, d=0x5E, E=0x79, F=0x71 입니다.

```c
const uint8_t HEX_FONT[16] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,
                              0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};

void seg_show(uint8_t pattern) {
    for (int i = 0; i < 8; i++)
        digitalWrite(SEG_PINS[i], (pattern ____ i) & 1);
}

void show_hex(int v, bool dot) {
    uint8_t p = HEX_FONT[v & ____];     // 하위 4비트만
    if (dot) p ____ 0x80;               // dp 비트 켜기
    seg_show(p);
}
```

<details><summary>🔒 정답 보기 1-1</summary>

```c
        digitalWrite(SEG_PINS[i], (pattern >> i) & 1);
    uint8_t p = HEX_FONT[v & 0x0F];
    if (dot) p |= 0x80;
```

`v & 0x0F` 는 하위 4비트만 남겨 0~15 로 만듭니다(배열 밖 접근 방지). `|=` 는 다른 비트를 건드리지 않고 한 비트만 켭니다 — Day 1 의 W1TS 와 같은 생각입니다.
</details>

#### ✏️ 괄호 넣기 1-2

1. 오늘 규칙에서 세그먼트 a 는 bit (　　　), dp 는 bit (　　　) 이다.
2. 숫자 0 의 비트맵은 0x(　　　) 이다.
3. 공통 양극(CA) 7-Segment 는 COM 을 (　　　)에 연결하고, 세그먼트에 (`HIGH` / `LOW`)를 주면 켜진다.
4. 비트맵을 뒤집는 연산자는 (　　　) 이다.

<details><summary>🔒 정답 보기 1-2</summary>

1. 0 / 7
2. 3F
3. 3V3 / LOW
4. ~
</details>

---

# 2교시 · 74HC595 — 선 3개로 출력 8개 (10:00–10:50)

**학습 목표**
- 직렬(한 줄로 한 비트씩) → 병렬(8비트 동시) 변환 원리를 설명한다
- DS · SH_CP(클럭) · ST_CP(래치)의 역할을 구분하고 `my_shift_out` 을 직접 구현한다
- MSB 먼저 / LSB 먼저의 차이를 안다

🎬 **D3-2 시프트 · 래치** — 비트를 하나씩 밀어 넣고 래치로 한 번에 출력하는 과정

<img src="img/d3_L2.svg" alt="2교시 배선">

| 595 핀 | 이름 | 오늘 연결 | 역할 |
|---|---|---|---|
| 14 | DS (SER) | GPIO23 | 넣을 비트 (0/1) |
| 11 | SH_CP (SRCLK) | GPIO18 | ↑ 에지마다 한 칸 밀기 |
| 12 | ST_CP (RCLK) | GPIO19 | ↑ 에지에 8비트를 출력 핀으로 복사 (래치) |
| 13 | OE | GND | LOW = 출력 켜짐 |
| 10 | MR | 3V3 | LOW = 전체 지우기 → 평소 HIGH |
| 15, 1~7 | Q0~Q7 | 7-Segment a~dp | 출력 |
| 9 | Q7' | (비움) | 다음 595 의 DS 로 — 여러 개 연결 |

### ▶ 예제 2-1 · my_shift_out — 비트를 하나씩 밀어 넣기

<img src="img/d3_ex2_1.svg" alt="예제 2-1 배선">

`shift595.ino`
```c
const int PIN_DS = 23, PIN_SH = 18, PIN_ST = 19;

void my_shift_out(uint8_t data) {          // MSB(bit7) 먼저
    for (int i = 7; i >= 0; i--) {
        digitalWrite(PIN_DS, (data >> i) & 1);   // 1) 비트를 DS 에 올리고
        digitalWrite(PIN_SH, HIGH);              // 2) 클럭 ↑ → 한 칸 밀림
        digitalWrite(PIN_SH, LOW);
    }
}

void write595(uint8_t data) {
    digitalWrite(PIN_ST, LOW);
    my_shift_out(data);
    digitalWrite(PIN_ST, HIGH);                  // 3) 래치 ↑ → 8개 출력이 동시에 바뀜
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_DS, OUTPUT); pinMode(PIN_SH, OUTPUT); pinMode(PIN_ST, OUTPUT);
}

void loop() {
    for (int i = 0; i < 8; i++) {
        write595(1 << i);                        // Q0 → Q7 하나씩
        Serial.printf("0x%02X\n", 1 << i);
        delay(400);
    }
}
// → 7-Segment 의 a, b, c, … dp 가 차례로 켜진다
```

MSB 먼저 8번 밀면 처음 넣은 bit7 이 맨 끝(Q7)까지 밀려가고, 마지막 bit0 이 Q0 에 남습니다. 래치를 올리기 전까지는 출력이 **바뀌지 않아서** 밀어 넣는 동안의 중간 모양이 보이지 않습니다.

### ▶ 예제 2-2 · 595 로 숫자 — 핀 3개로 1교시와 같은 결과

<img src="img/d3_ex2_2.svg" alt="예제 2-2 배선">

`seg595.ino`
```c
const int PIN_DS = 23, PIN_SH = 18, PIN_ST = 19;
const uint8_t DIGITS[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

void write595(uint8_t data) {
    digitalWrite(PIN_ST, LOW);
    shiftOut(PIN_DS, PIN_SH, MSBFIRST, data);    // Arduino 내장 함수 — my_shift_out 과 같다
    digitalWrite(PIN_ST, HIGH);
}

void setup() {
    pinMode(PIN_DS, OUTPUT); pinMode(PIN_SH, OUTPUT); pinMode(PIN_ST, OUTPUT);
}

void loop() {
    for (int d = 0; d < 10; d++) {
        write595(DIGITS[d]);
        delay(600);
    }
}
```

1교시는 GPIO 8개, 지금은 **3개**입니다. 595 를 Q7' → 다음 595 DS 로 이으면 **같은 3개로 16, 24 … 개**를 제어합니다(3교시 4자리, 4교시 매트릭스에서 활용).

### ▶ 예제 2-3 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| 래치(ST_CP)를 안 올림 | 아무리 보내도 출력이 안 바뀜 |
| MR 을 연결하지 않음(떠 있음) | 가끔 전부 지워짐 → **3V3 에 고정** |
| OE 를 연결하지 않음 | 출력이 켜졌다 꺼졌다 → **GND 에 고정** |
| 595 를 거꾸로 꽂음 (홈이 오른쪽) | VCC 와 GND 가 뒤바뀜 → **뜨거워짐, 즉시 USB 분리** |
| `LSBFIRST` 로 보냄 | 비트 순서가 뒤집혀 모양이 거울처럼 바뀜 (빈칸 2-1) |

#### ✏️ 빈칸 채우기 2-1

LSB(bit0)를 먼저 보내는 버전입니다. 같은 모양이 나오려면 배선에서 Q0 와 Q7 순서가 반대여야 합니다.

```c
void my_shift_out_lsb(uint8_t data) {
    for (int i = ____; i < 8; i++) {
        digitalWrite(PIN_DS, (data >> ____) & 1);
        digitalWrite(PIN_SH, ____);
        digitalWrite(PIN_SH, LOW);
    }
}

void write595(uint8_t data) {
    digitalWrite(PIN_ST, LOW);
    my_shift_out_lsb(data);
    digitalWrite(____, HIGH);
}
```

<details><summary>🔒 정답 보기 2-1</summary>

```c
    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_DS, (data >> i) & 1);
        digitalWrite(PIN_SH, HIGH);
    digitalWrite(PIN_ST, HIGH);
```

LSB 먼저 보내면 처음 넣은 bit0 이 Q7 까지 밀려갑니다. `shiftOut(…, LSBFIRST, …)` 와 같습니다.
</details>

#### ✏️ 괄호 넣기 2-2

1. 595 에서 클럭 핀 (　　　)이 올라갈 때마다 비트가 한 칸 밀린다.
2. 밀어 넣은 8비트를 출력 핀으로 한 번에 복사하는 핀은 (　　　)이다.
3. OE 는 (`GND` / `3V3`), MR 은 (`GND` / `3V3`)에 연결한다.
4. 595 여러 개를 이을 때는 앞 595 의 (　　　) 핀을 다음 595 의 DS 에 연결한다.

<details><summary>🔒 정답 보기 2-2</summary>

1. SH_CP (SRCLK)
2. ST_CP (RCLK, 래치)
3. GND / 3V3
4. Q7'
</details>

---

# 3교시 · 4자리 7-Segment — 멀티플렉싱과 타이머 (11:00–11:50)

**학습 목표**
- 한 순간에 한 자리만 켜고 빠르게 돌려 네 자리가 동시에 보이게 한다(잔상)
- 하드웨어 타이머 인터럽트로 loop 와 상관없이 화면을 갱신한다
- 숫자를 `% 10`, `/ 10` 으로 자릿수로 나눈다

🎬 **D3-3 멀티플렉싱** — 갱신 속도를 낮춰 한 자리씩 켜지는 것을 보고, 올리면 네 자리가 동시에 보이는 것 확인

<img src="img/d3_L3.svg" alt="3교시 배선">

4자리 모듈은 **세그먼트 a~dp 8개를 네 자리가 공유**하고, 자리마다 공통 단자(D1~D4)가 따로 있습니다. 공통 음극형이면 **켤 자리의 D 만 LOW**, 나머지는 HIGH 로 둡니다. 세그먼트 8개에는 각각 220Ω 을 둡니다.

> 한 자리가 켜질 때 그 자리의 D 핀으로 최대 8개 세그먼트 전류가 모입니다. 한 자리에 머무는 시간은 짧게(수 ms) — **한 자리만 켜 둔 채 멈추지 않도록** 합니다.

### ▶ 예제 3-1 · loop 로 멀티플렉싱 — 그리고 한계

<img src="img/d3_ex3_1.svg" alt="예제 3-1 배선">

`mux_loop.ino`
```c
const int PIN_DS = 23, PIN_SH = 18, PIN_ST = 19;
const int DIG[4] = {25, 26, 27, 14};                 // D1 ~ D4
const uint8_t DIGITS[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
uint8_t disp[4] = {DIGITS[1], DIGITS[2], DIGITS[3], DIGITS[4]};   // "1234"

void write595(uint8_t d) {
    digitalWrite(PIN_ST, LOW);
    shiftOut(PIN_DS, PIN_SH, MSBFIRST, d);
    digitalWrite(PIN_ST, HIGH);
}

void show_digit(int k) {
    for (int i = 0; i < 4; i++) digitalWrite(DIG[i], HIGH);   // 전부 끄고 (CC: HIGH = 꺼짐)
    write595(disp[k]);                                        // 모양을 바꾼 뒤
    digitalWrite(DIG[k], LOW);                                // 그 자리만 켠다
}

void setup() {
    pinMode(PIN_DS, OUTPUT); pinMode(PIN_SH, OUTPUT); pinMode(PIN_ST, OUTPUT);
    for (int i = 0; i < 4; i++) pinMode(DIG[i], OUTPUT);
}

void loop() {
    for (int k = 0; k < 4; k++) {
        show_digit(k);
        delay(3);                     // 한 자리 3ms → 4자리 12ms → 약 83Hz
    }
    // delay(100);                    // ← 이 줄을 켜면? 한 자리만 오래 켜져 깜빡인다
}
```

사람 눈은 약 **50Hz 이상** 깜빡임을 느끼지 못합니다. 문제는 loop 에서 **다른 일(센서 읽기, delay)** 을 하면 갱신이 멈춘다는 것입니다. 주석의 `delay(100)` 을 켜 보세요 — 다음 예제에서 해결합니다.

### ▶ 예제 3-2 · 타이머 인터럽트 — loop 가 바빠도 화면은 돈다

<img src="img/d3_ex3_2.svg" alt="예제 3-2 배선">

`mux_timer.ino`
```c
const int PIN_DS = 23, PIN_SH = 18, PIN_ST = 19;
const int DIG[4] = {25, 26, 27, 14};
const uint8_t DIGITS[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

volatile uint8_t disp[4];               // ISR 과 loop 가 함께 쓰는 화면 버퍼
volatile int cur = 0;
hw_timer_t *timer = NULL;

void IRAM_ATTR write595(uint8_t d) {
    digitalWrite(PIN_ST, LOW);
    for (int i = 7; i >= 0; i--) {
        digitalWrite(PIN_DS, (d >> i) & 1);
        digitalWrite(PIN_SH, HIGH);
        digitalWrite(PIN_SH, LOW);
    }
    digitalWrite(PIN_ST, HIGH);
}

void IRAM_ATTR onTimer() {              // 2ms 마다 한 자리씩
    digitalWrite(DIG[cur], HIGH);       // 지금 자리 끄고
    cur = (cur + 1) % 4;
    write595(disp[cur]);
    digitalWrite(DIG[cur], LOW);        // 다음 자리 켜기
}

void show_number(int n) {
    for (int k = 3; k >= 0; k--) {      // 오른쪽 자리부터
        disp[k] = DIGITS[n % 10];
        n /= 10;
    }
}

void setup() {
    pinMode(PIN_DS, OUTPUT); pinMode(PIN_SH, OUTPUT); pinMode(PIN_ST, OUTPUT);
    for (int i = 0; i < 4; i++) { pinMode(DIG[i], OUTPUT); digitalWrite(DIG[i], HIGH); }
    timer = timerBegin(1000000);                   // 1MHz → 1틱 = 1µs
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 2000, true, 0);              // 2000µs 마다 반복
}

void loop() {
    static int count = 0;
    show_number(count++ % 10000);
    delay(100);                          // loop 가 쉬어도 화면은 깜빡이지 않는다
}
```

Day 1 6교시 `attachInterrupt` 는 **핀의 변화**가 ISR 을 불렀고, 여기서는 **시간(타이머)** 이 ISR 을 부릅니다. 규칙은 같습니다: ISR 은 `IRAM_ATTR`, 짧게, 공유 변수는 `volatile`. `timerBegin` · `timerAlarm` 은 **core 3.x** 방식이며, 인터넷의 `timerBegin(0, 80, true)` 예제는 2.x 방식이라 컴파일되지 않습니다.

### ▶ 예제 3-3 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| 자리를 바꾸기 전에 이전 자리를 끄지 않음 | 옆 자리에 숫자가 **희미하게 겹쳐** 보임(고스팅) — 끄기 → 모양 바꾸기 → 켜기 순서 |
| 한 자리 머무는 시간 20ms | 4자리 80ms → 12Hz → 눈에 띄게 깜빡임 |
| ISR 에서 `Serial.printf`, `delay` | 느려지거나 리셋 (Day 1 6교시 규칙) |
| 2.x 예제 `timerBegin(0, 80, true)` | `too many arguments` 컴파일 에러 → 3.x API |
| CA 모듈인데 D 를 LOW 로 켬 | 켤 자리만 꺼지는 반대 동작 → HIGH/LOW 와 비트맵 모두 반전 |

#### ✏️ 빈칸 채우기 3-1

숫자를 자릿수로 나누되, **앞자리의 0 은 끄기**(예: 42 → "  42")를 합니다.

```c
void show_number(int n) {
    for (int k = 3; k >= 0; k--) {
        if (n == 0 && k < 3) disp[k] = ____;        // 앞자리 0 은 빈칸
        else disp[k] = DIGITS[n ____ 10];
        n ____ 10;
    }
}
```

<details><summary>🔒 정답 보기 3-1</summary>

```c
        if (n == 0 && k < 3) disp[k] = 0x00;
        else disp[k] = DIGITS[n % 10];
        n /= 10;
```

`k < 3` 조건 덕분에 숫자 0 은 맨 오른쪽 한 자리에 "0" 으로 남습니다. `n % 10` 은 일의 자리, `n /= 10` 은 한 자리 오른쪽으로 밀기 — 비트 시프트의 10진수 버전입니다.
</details>

#### ✏️ 괄호 넣기 3-2

1. 한 순간에 한 자리만 켜고 빠르게 돌려 모두 켜진 것처럼 보이게 하는 방식을 (　　　)이라 한다.
2. core 3.x 에서 1µs 단위 타이머를 만드는 호출은 `timerBegin(　　　)` 이다.
3. ISR 과 loop 가 함께 쓰는 화면 버퍼에는 (　　　) 를 붙인다.
4. 옆 자리가 희미하게 겹쳐 보이는 현상을 (　　　)이라 하며, 자리를 바꾸기 전에 이전 자리를 (　　　) 막는다.

<details><summary>🔒 정답 보기 3-2</summary>

1. 멀티플렉싱 (다이내믹 구동)
2. 1000000
3. volatile
4. 고스팅 / 꺼서
</details>

---

# 4교시 · 8×8 도트 매트릭스 — 2차원 비트맵 (13:00–13:50)

**학습 목표**
- 8바이트 배열로 8×8 그림을 표현한다 (바이트 = 행, 비트 = 열)
- 행 스캔 멀티플렉싱을 타이머로 구현한다
- 비트 시프트로 그림을 옆으로 스크롤한다

🎬 **D3-4 매트릭스 편집기** — 점을 눌러 그림을 그리면 8개의 행 바이트가 만들어지고, 스크롤을 재생

<img src="img/d3_L4.svg" alt="4교시 배선">

> **1088AS 핀 번호** — 16핀이 행 · 열 순서로 나열되어 있지 않습니다. 부품 옆면의 **1번 핀 표시**를 찾고 아래 표대로 연결합니다. 모듈이 커서 브레드보드에 잘 맞지 않으면 **M-F 선으로 직접** 연결해도 됩니다. 제품에 따라 행 · 열의 극성이 반대일 수 있으니, 예제 4-1 이 뒤집혀 보이면 코드의 `ROW_ON` 을 바꿉니다.

| 핀 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 신호 | R5 | R7 | C2 | C3 | R8 | C5 | R6 | R3 | R1 | C4 | C6 | R4 | C1 | R2 | C7 | C8 |

행 R1~R8 은 GPIO 13 · 14 · 27 · 26 · 25 · 33 · 32 · 4, 열 C1~C8 은 **220Ω 을 거쳐** 595 의 Q0~Q7 입니다.

### ▶ 예제 4-1 · 하트 — 한 행씩 빠르게

<img src="img/d3_ex4_1.svg" alt="예제 4-1 배선">

`matrix_heart.ino`
```c
const int PIN_DS = 23, PIN_SH = 18, PIN_ST = 19;
const int ROWS[8] = {13, 14, 27, 26, 25, 33, 32, 4};
const int ROW_ON = HIGH;                 // 행 극성 — 뒤집혀 보이면 LOW 로

volatile uint8_t img[8] = {              // 1 = 켜짐, bit7 = 왼쪽 열
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000,
};
volatile int row = 0;
hw_timer_t *timer = NULL;

void IRAM_ATTR write595(uint8_t d) {
    digitalWrite(PIN_ST, LOW);
    for (int i = 0; i < 8; i++) {                 // bit7 → Q0(C1, 왼쪽)
        digitalWrite(PIN_DS, (d >> i) & 1);
        digitalWrite(PIN_SH, HIGH);
        digitalWrite(PIN_SH, LOW);
    }
    digitalWrite(PIN_ST, HIGH);
}

void IRAM_ATTR onTimer() {
    digitalWrite(ROWS[row], !ROW_ON);             // 이전 행 끄기
    row = (row + 1) % 8;
    write595(ROW_ON == HIGH ? ~img[row] : img[row]);   // 열은 행과 반대 극성일 때 켜짐
    digitalWrite(ROWS[row], ROW_ON);              // 다음 행 켜기
}

void setup() {
    pinMode(PIN_DS, OUTPUT); pinMode(PIN_SH, OUTPUT); pinMode(PIN_ST, OUTPUT);
    for (int i = 0; i < 8; i++) { pinMode(ROWS[i], OUTPUT); digitalWrite(ROWS[i], !ROW_ON); }
    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 1500, true, 0);             // 1.5ms × 8행 = 12ms → 약 83Hz
}

void loop() { }
```

한 행 안에서 점이 켜지려면 **행은 한쪽 극성, 열은 반대 극성**이어야 LED 에 전류가 흐릅니다. 그래서 행을 HIGH 로 켜는 제품은 열 데이터를 `~` 로 뒤집어 보냅니다. 3교시의 "자리"가 여기서는 "행"입니다.

### ▶ 예제 4-2 · 글자 스크롤 — 비트 시프트

<img src="img/d3_ex4_2.svg" alt="예제 4-2 배선">

`matrix_scroll.ino`
```c
// 예제 4-1 에서 PIN_DS ~ setup() 끝까지를 그대로 쓰고 loop 만 바꾼다
const uint8_t FONT_H[8] = {0x00,0x66,0x66,0x7E,0x66,0x66,0x66,0x00};   // H
const uint8_t FONT_I[8] = {0x00,0x3C,0x18,0x18,0x18,0x18,0x3C,0x00};   // I

void loop() {
    // H 와 I 를 16비트로 이어 붙이고 왼쪽으로 한 칸씩 민다
    for (int shift = 0; shift <= 8; shift++) {
        for (int r = 0; r < 8; r++) {
            uint16_t line = (FONT_H[r] << 8) | FONT_I[r];   // 16비트 한 줄
            img[r] = (line << shift) >> 8;                  // 위 8비트만 보이는 창
        }
        delay(150);
    }
    delay(800);
}
```

`(FONT_H[r] << 8) | FONT_I[r]` 는 두 바이트를 이어 붙인 **16비트 띠**이고, `<< shift` 로 띠를 왼쪽으로 밀면서 `>> 8` 로 **위 8비트 창**만 봅니다. 여러 글자면 더 긴 띠(배열)를 쓰면 됩니다.

### ▶ 예제 4-3 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| 핀 번호를 1~8 = 행으로 착각 | 점이 흩어져 엉뚱한 모양 → 핀 표 확인 |
| 모양이 위아래 / 좌우 뒤집힘 | 행 배열 순서 또는 비트 순서 반대 → `ROWS[]` 순서, `bit7 = 왼쪽` 확인 |
| 모든 점이 켜지고 그림 부분만 꺼짐 | 극성 반대 → `ROW_ON` 변경 |
| 열에 저항 없이 연결 | 과전류 — 열마다 220Ω |

#### ✏️ 빈칸 채우기 4-1

(x, y) 점 하나를 켜고 끄는 함수입니다. x = 열(0 = 왼쪽), y = 행(0 = 위).

```c
void set_pixel(int x, int y, bool on) {
    if (x < 0 || x > 7 || y < 0 || y > 7) return;   // 범위 밖 무시
    uint8_t mask = 0x80 ____ x;                      // x=0 → bit7
    if (on) img[y] ____ mask;
    else    img[y] &= ____mask;
}
```

<details><summary>🔒 정답 보기 4-1</summary>

```c
    uint8_t mask = 0x80 >> x;
    if (on) img[y] |= mask;
    else    img[y] &= ~mask;
```

켜기는 `|= mask`, 끄기는 `&= ~mask` — Day 1 레지스터 조작과 완전히 같은 두 줄입니다.
</details>

#### ✏️ 괄호 넣기 4-2

1. 8×8 그림은 (　　　)바이트 배열이며, 한 바이트가 한 (`행` / `열`)이다.
2. 한 행씩 빠르게 돌아가며 켜는 방식은 3교시의 (　　　)과 같다.
3. 점 하나를 끄려면 `img[y] &= (　　　)mask` 를 쓴다.
4. 두 글자를 이어 붙인 16비트 띠를 왼쪽으로 밀어 스크롤할 때 쓰는 연산자는 (　　　) 이다.

<details><summary>🔒 정답 보기 4-2</summary>

1. 8 / 행
2. 멀티플렉싱
3. ~
4. <<
</details>

---

# 5교시 · I2C 와 1602 LCD (14:00–14:50)

**학습 목표**
- I2C 의 두 선(SDA · SCL), 주소, 풀업 저항을 설명한다
- I2C 스캐너로 장치 주소를 찾는다
- LCD 에 센서 값을 **고정 폭**으로 출력한다

🎬 **D3-5 I2C 버스** — 시작 · 주소 · ACK · 데이터 · 정지 비트가 두 선에 흐르는 모습, 스캐너가 주소를 찾는 과정

<img src="img/d3_L5.svg" alt="5교시 배선">

I2C 는 **SDA(데이터) · SCL(클럭) 두 선**에 여러 장치를 달고, 장치마다 **7비트 주소**로 구분합니다. ESP32 의 기본 I2C 핀은 **SDA 21 · SCL 22** 입니다. LCD 는 커서 브레드보드 대신 **M-F 선 4개**로 백팩 핀에 직접 연결해도 됩니다.

> **LCD 전원** — 먼저 **3V3** 로 연결합니다. 글자가 안 보이거나 흐리면 백팩 뒤 **파란 가변저항을 드라이버로 돌려 대비**를 맞춥니다. 그래도 안 보이는 제품은 5V 가 필요하며, 이때 백팩의 풀업 저항 때문에 SDA · SCL 이 5V 로 올라간다는 점을 강사와 확인합니다.

### ▶ 예제 5-1 · I2C 스캐너 — 누가 연결되어 있나

<img src="img/d3_ex5_1.svg" alt="예제 5-1 배선">

`i2c_scan.ino`
```c
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);                     // SDA, SCL
}

void loop() {
    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();     // 0 = 응답(ACK) 있음
        if (err == 0) {
            Serial.printf("발견: 0x%02X\n", addr);
            found++;
        }
    }
    Serial.printf("총 %d개\n\n", found);
    delay(3000);
}
// → 발견: 0x27   (또는 0x3F)
```

`endTransmission()` 반환값: **0 = 성공**, 2 = 주소에 응답 없음(NACK), 그 밖은 버스 오류입니다. 아무것도 안 나오면 SDA · SCL 이 바뀌었거나 전원이 빠진 것입니다.

### ▶ 예제 5-2 · LCD 에 글자

<img src="img/d3_ex5_2.svg" alt="예제 5-2 배선">

준비의 **따라하기 · LiquidCrystal I2C 라이브러리 설치** 를 먼저 합니다.

`lcd_hello.ino`
```c
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);        // 주소 · 16칸 · 2줄 — 스캐너 결과로

void setup() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);                   // (열, 행) — 0부터
    lcd.print("Hello ESP32");
    lcd.setCursor(0, 1);
    lcd.print("Day 3 I2C LCD");
}

void loop() {
    lcd.setCursor(13, 1);
    lcd.printf("%3lu", (millis() / 1000) % 1000);   // 오른쪽 아래에 초 카운트
    delay(200);
}
```

### ▶ 예제 5-3 · 센서 값을 LCD 에 — 고정 폭으로

<img src="img/d3_ex5_3.svg" alt="예제 5-3 배선">

`lcd_sensor.ino`
```c
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int PIN_POT = 34;

byte DEG[8] = {0b01100, 0b10010, 0b10010, 0b01100, 0, 0, 0, 0};   // ° 모양 5×8

void setup() {
    lcd.init();
    lcd.backlight();
    lcd.createChar(0, DEG);                // 사용자 문자 0번
}

void loop() {
    int raw = analogRead(PIN_POT);
    char line[17];                         // 16글자 + '\0'
    snprintf(line, sizeof(line), "pot:%4d %3d%%", raw, raw * 100 / 4095);
    lcd.setCursor(0, 0);
    lcd.print(line);

    float temp = 24.6;                     // Day 2 LM35 값이라고 가정
    snprintf(line, sizeof(line), "T:%5.1f", temp);
    lcd.setCursor(0, 1);
    lcd.print(line);
    lcd.write(0);                          // ° 출력
    lcd.print("C   ");
    delay(200);
}
```

LCD 는 **덮어쓴 칸만 바뀝니다**. "1000" 다음에 "999" 를 쓰면 마지막 "0" 이 남아 "9990" 처럼 보입니다. `%4d` 처럼 **폭을 고정**하면 남는 글자가 생기지 않습니다. `lcd.clear()` 는 느리고 깜빡이므로 loop 에서 자주 부르지 않습니다. `createChar` 로 °, 화살표 같은 5×8 문자를 8개까지 만들 수 있습니다 — 4교시 비트맵과 같은 원리입니다.

### ▶ 예제 5-4 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| 백라이트만 켜지고 글자 없음 | **대비 가변저항** 조정 · 주소(0x27 ↔ 0x3F) 확인 |
| SDA · SCL 을 바꿔 꽂음 | 스캐너가 아무것도 못 찾음 |
| loop 마다 `lcd.clear()` | 화면이 깜빡이고 느림 → 고정 폭으로 덮어쓰기 |
| `char line[16]` 에 16글자 | `'\0'` 자리가 없어 넘침 → 17칸 (C 과정 Day 2 문자열) |
| I2C 선이 길고 늘어짐 | 가끔 글자 깨짐 — 선은 짧게, 느슨한 연결 확인 |

#### ✏️ 빈칸 채우기 5-1

두 값을 한 줄에 고정 폭으로 출력합니다: `"L:1733 P:2041  "`.

```c
void lcd_line(int row, int light, int pot) {
    char buf[____];                                   // 16글자 + 1
    snprintf(buf, ____, "L:%4d P:%4d  ", light, pot);
    lcd.setCursor(____, row);
    lcd.print(____);
}
```

<details><summary>🔒 정답 보기 5-1</summary>

```c
    char buf[17];
    snprintf(buf, sizeof(buf), "L:%4d P:%4d  ", light, pot);
    lcd.setCursor(0, row);
    lcd.print(buf);
```

`snprintf` 는 크기를 넘기면 잘라 주므로 배열을 넘치지 않습니다. `sizeof(buf)` 는 배열일 때만 17 입니다(포인터면 4).
</details>

#### ✏️ 괄호 넣기 5-2

1. I2C 는 (　　　) · (　　　) 두 선을 쓰며, ESP32 기본 핀은 21 · 22 이다.
2. I2C 장치는 (　　　)비트 주소로 구분한다.
3. `Wire.endTransmission()` 이 (　　　)을 돌려주면 장치가 응답한 것이다.
4. LCD 에 남는 글자를 막으려면 `%4d` 처럼 (　　　)을 고정한다.

<details><summary>🔒 정답 보기 5-2</summary>

1. SDA / SCL
2. 7
3. 0
4. 폭
</details>

---

# 6교시 · PCF8591 — 라이브러리 없이 I2C 직접 (15:00–15:50)

**학습 목표**
- 데이터시트의 제어 바이트를 비트 연산으로 만든다
- `Wire` 의 쓰기 · 읽기 순서로 ADC 값을 읽고 DAC 로 전압을 낸다
- 장치 드라이버를 구조체 + 함수로 설계하고 오류를 반환한다

🎬 **D3-6 제어 바이트** — 비트를 켜고 끄며 제어 바이트를 만들고, 읽기 첫 바이트가 "이전 값"인 이유 확인

<img src="img/d3_L6.svg" alt="6교시 배선">

PCF8591 은 **8비트 ADC 4채널(AIN0~3) + 8비트 DAC 1채널(AOUT)** 을 가진 I2C 칩입니다. LCD 와 **같은 SDA · SCL** 에 연결합니다(모듈끼리 점퍼로 이어 한 버스). 오늘은 AIN0 에 가변저항, AOUT 에 LED 를 연결합니다.

**제어 바이트** (데이터시트)

| 비트 | 7 | 6 | 5 · 4 | 3 | 2 | 1 · 0 |
|---|---|---|---|---|---|---|
| 의미 | 0 | **DAC 출력 켜기** | 입력 방식 (00 = 단일 4채널) | 0 | 자동 증가 | **채널 번호** |

예: AIN0 읽기 + DAC 켜기 = `0100 0000` = **0x40**, AIN2 = `0x42`.

### ▶ 예제 6-1 · ADC 읽기 — 첫 바이트는 버린다

<img src="img/d3_ex6_1.svg" alt="예제 6-1 배선">

`pcf_adc.ino`
```c
#include <Wire.h>
const uint8_t PCF = 0x48;

int pcf_read(uint8_t ch) {
    Wire.beginTransmission(PCF);
    Wire.write(0x40 | (ch & 0x03));          // DAC 켜기 + 채널
    if (Wire.endTransmission() != 0) return -1;
    Wire.requestFrom(PCF, (uint8_t)2);        // 2바이트 요청
    Wire.read();                              // 첫 바이트 = 이전 변환 값 → 버림
    return Wire.read();                       // 0 ~ 255
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
}

void loop() {
    int v = pcf_read(0);
    if (v < 0) Serial.println("PCF8591 응답 없음");
    else Serial.printf("ain0:%d,esp_adc:%d\n", v, analogRead(34) / 16);
    delay(100);
}
// → 가변저항을 돌리면 0 ~ 255
```

PCF8591 은 **요청을 받는 순간 변환을 시작**하므로 첫 번째로 돌려주는 바이트는 **지난번 변환 결과**입니다. 그래서 2바이트를 요청해 첫 바이트를 버립니다. 8비트(0~255)라 ESP32 ADC(12비트) 보다 거칩니다 — 외부 ADC 는 핀이 모자랄 때 씁니다.

### ▶ 예제 6-2 · DAC — 숫자로 전압을 낸다

<img src="img/d3_ex6_2.svg" alt="예제 6-2 배선">

`pcf_dac.ino`
```c
#include <Wire.h>
const uint8_t PCF = 0x48;

bool pcf_write_dac(uint8_t value) {
    Wire.beginTransmission(PCF);
    Wire.write(0x40);                         // DAC 켜기
    Wire.write(value);                        // 0 ~ 255 → 0 ~ VCC
    return Wire.endTransmission() == 0;
}

void setup() { Wire.begin(21, 22); }

void loop() {
    for (int v = 0; v < 256; v += 4)   { pcf_write_dac(v); delay(15); }
    for (int v = 255; v >= 0; v -= 4) { pcf_write_dac(v); delay(15); }
}
// → LED 가 부드럽게 밝아졌다 어두워진다 — PWM 이 아니라 진짜 전압
```

Day 1 7교시의 PWM 은 **빠르게 켜고 끄기**, DAC 는 **실제로 중간 전압**을 냅니다. LED 는 약 1.8V 이하에서는 켜지지 않으므로 v 가 작을 때는 어둡게 머뭅니다.

### ▶ 예제 6-3 · 두 장치 한 버스 — 드라이버를 구조체로

<img src="img/d3_ex6_3.svg" alt="예제 6-3 배선">

`pcf_lcd.ino`
```c
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

typedef struct {
    uint8_t addr;
    uint8_t last[4];        // 채널별 마지막 값
    int errors;             // 통신 실패 횟수
} pcf8591_t;

int pcf_read(pcf8591_t *p, uint8_t ch) {
    Wire.beginTransmission(p->addr);
    Wire.write(0x40 | (ch & 0x03));
    if (Wire.endTransmission() != 0) { p->errors++; return -1; }
    if (Wire.requestFrom(p->addr, (uint8_t)2) != 2) { p->errors++; return -1; }
    Wire.read();
    p->last[ch & 0x03] = Wire.read();
    return p->last[ch & 0x03];
}

pcf8591_t pcf = {0x48, {0}, 0};
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    Wire.begin(21, 22);
    lcd.init();
    lcd.backlight();
}

void loop() {
    int v = pcf_read(&pcf, 0);
    char buf[17];
    snprintf(buf, sizeof(buf), "AIN0:%3d err:%2d", v < 0 ? 0 : v, pcf.errors);
    lcd.setCursor(0, 0);
    lcd.print(buf);
    delay(100);
}
```

주소가 다르니(0x27, 0x48) 한 버스에서 충돌하지 않습니다. `pcf8591_t` 에 주소와 상태를 넣어 두면 PCF8591 을 두 개(주소 0x48, 0x49) 달아도 **변수만 하나 더** 만들면 됩니다 — Day 2 `powered_sensor_t` 와 같은 설계입니다.

### ▶ 예제 6-4 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| `requestFrom(…, 1)` 로 한 바이트만 | 항상 **한 박자 늦은 값** — 2바이트 읽고 첫 바이트 버림 |
| 제어 바이트에 0x40 을 안 넣음 | DAC 가 꺼져 AOUT 이 0V |
| `Wire.begin()` 을 LCD 쪽과 따로 두 번 다른 핀으로 | 버스가 꼬임 → 한 번만, 21 · 22 |
| 반환값 검사 없이 사용 | 선이 빠져도 255 나 이전 값이 그대로 나와 **고장을 모름** |

#### ✏️ 빈칸 채우기 6-1

`pcf_read` 를 완성합니다.

```c
int pcf_read(pcf8591_t *p, uint8_t ch) {
    Wire.beginTransmission(p->____);
    Wire.write(0x40 | (ch & ____));
    if (Wire.endTransmission() != 0) { p->errors++; return -1; }
    Wire.requestFrom(p->addr, (uint8_t)____);
    Wire.read();                          // 이전 값 버림
    return Wire.____();
}
```

<details><summary>🔒 정답 보기 6-1</summary>

```c
    Wire.beginTransmission(p->addr);
    Wire.write(0x40 | (ch & 0x03));
    Wire.requestFrom(p->addr, (uint8_t)2);
    return Wire.read();
```
</details>

#### ✏️ 괄호 넣기 6-2

1. PCF8591 의 기본 I2C 주소는 0x(　　　) 이다.
2. 제어 바이트에서 DAC 출력을 켜는 비트는 bit (　　　) 이다.
3. PCF8591 에서 읽은 첫 바이트는 (　　　) 변환 결과라서 버린다.
4. PCF8591 의 ADC · DAC 는 (　　　)비트이다.

<details><summary>🔒 정답 보기 6-2</summary>

1. 48
2. 6
3. 이전 (지난번)
4. 8
</details>

---

# 7교시 · DC 모터(L293D)와 릴레이 (16:00–16:50)

**학습 목표**
- 모터를 GPIO 에 직접 연결하면 안 되는 이유와 H-브리지의 원리를 설명한다
- IN1 · IN2 로 방향, EN 의 PWM 으로 속도를 제어한다
- 릴레이로 큰 부하를 켜고 끄는 원리와 안전 규칙을 안다

🎬 **D3-7 H-브리지** — IN1 · IN2 · PWM 을 바꾸며 전류 경로와 모터 회전 방향 · 속도 확인

<img src="img/d3_L7.svg" alt="7교시 배선">

> **모터는 절대 GPIO 에 직접 연결하지 않습니다.** 모터는 수백 mA 가 필요하고(GPIO 는 20mA), 멈출 때 **역전압**이 생깁니다. L293D 는 ESP32 의 작은 신호를 받아 **5V 전원의 큰 전류를 대신 흘려 주는** 드라이버이며, 역전압 보호 다이오드도 들어 있습니다.
>
> **USB 5V 로 모터를 돌리면** 시작 순간 전압이 떨어져 ESP32 가 리셋될 수 있습니다(`Brownout detector was triggered`). 그러면 모터를 천천히 가속하거나(예제 7-2), 강사가 외부 5V 전원을 연결합니다(GND 는 공통).

| L293D 핀 | 이름 | 연결 |
|---|---|---|
| 1 | EN1 | GPIO25 — PWM 속도 |
| 2 · 7 | IN1 · IN2 | GPIO26 · GPIO27 — 방향 |
| 3 · 6 | OUT1 · OUT2 | 모터 두 선 |
| 4 · 5 · 12 · 13 | GND | GND (방열도 겸함) |
| 8 | VS (모터 전원) | 5V |
| 16 | VSS (논리 전원) | 5V |

### ▶ 예제 7-1 · H-브리지 — IN1 · IN2 로 방향

<img src="img/d3_ex7_1.svg" alt="예제 7-1 배선">

| IN1 | IN2 | 모터 |
|---|---|---|
| HIGH | LOW | 정방향 |
| LOW | HIGH | 역방향 |
| LOW | LOW | 자유 정지 (관성으로 돈다) |
| HIGH | HIGH | 브레이크 (빨리 멈춘다) |

`motor_dir.ino`
```c
const int PIN_EN = 25, PIN_IN1 = 26, PIN_IN2 = 27;

void motor_dir(int in1, int in2, const char *msg) {
    digitalWrite(PIN_IN1, in1);
    digitalWrite(PIN_IN2, in2);
    Serial.println(msg);
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_EN, OUTPUT); pinMode(PIN_IN1, OUTPUT); pinMode(PIN_IN2, OUTPUT);
    digitalWrite(PIN_EN, HIGH);              // 속도 최대 (PWM 은 다음 예제)
}

void loop() {
    motor_dir(HIGH, LOW,  "정방향");  delay(2000);
    motor_dir(LOW,  LOW,  "정지");    delay(1000);      // 반대로 돌리기 전에 멈춘다
    motor_dir(LOW,  HIGH, "역방향");  delay(2000);
    motor_dir(HIGH, HIGH, "브레이크"); delay(1000);
}
```

### ▶ 예제 7-2 · PWM 속도와 부드러운 가속

<img src="img/d3_ex7_2.svg" alt="예제 7-2 배선">

`motor_speed.ino`
```c
const int PIN_EN = 25, PIN_IN1 = 26, PIN_IN2 = 27;

void motor_set(int speed) {                  // -255 ~ +255, 부호 = 방향
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;
    digitalWrite(PIN_IN1, speed > 0);
    digitalWrite(PIN_IN2, speed < 0);
    ledcWrite(PIN_EN, abs(speed));
}

void ramp_to(int *cur, int target) {         // 한 번에 조금씩
    while (*cur != target) {
        *cur += (target > *cur) ? 5 : -5;
        if (abs(target - *cur) < 5) *cur = target;
        motor_set(*cur);
        delay(20);
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_IN1, OUTPUT); pinMode(PIN_IN2, OUTPUT);
    ledcAttach(PIN_EN, 20000, 8);            // 20kHz — 귀에 거슬리는 소리가 안 들린다
}

void loop() {
    static int speed = 0;
    ramp_to(&speed, 200);  delay(1500);
    ramp_to(&speed, 0);    delay(500);
    ramp_to(&speed, -200); delay(1500);
    ramp_to(&speed, 0);    delay(500);
}
```

속도를 **부호 있는 정수 하나**로 표현하면(양수 = 정방향, 음수 = 역방향) 호출하는 쪽이 간단해집니다. `ramp_to` 는 현재 속도를 **포인터로 받아 갱신**합니다. 모터는 duty 가 너무 낮으면(약 80 이하) 윙 소리만 나고 돌지 않습니다 — 최소 구동값을 실험으로 찾아 보세요.

### ▶ 예제 7-3 · 릴레이 — 작은 신호로 큰 스위치

<img src="img/d3_ex7_3.svg" alt="예제 7-3 배선">

릴레이는 **전자석으로 금속 접점을 움직이는 스위치**입니다. ESP32 와 부하(램프, 펌프)가 **전기적으로 분리**됩니다. 모듈에는 코일을 구동하는 트랜지스터와 역전압 다이오드가 들어 있습니다.

`relay.ino`
```c
const int PIN_RELAY = 13;
const bool ACTIVE_LOW = false;               // 모듈에 따라 LOW 에서 켜진다

void relay_set(bool on) {
    digitalWrite(PIN_RELAY, ACTIVE_LOW ? !on : on);
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_RELAY, OUTPUT);
    relay_set(false);
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == '1') { relay_set(true);  Serial.println("ON");  }
        if (c == '0') { relay_set(false); Serial.println("OFF"); }
    }
}
// → 시리얼 모니터에서 1 / 0 을 보내면 딸깍 소리와 함께 모듈 LED 가 켜지고 꺼진다
```

접점 단자는 **COM**(공통), **NO**(평소 열림 — 켜면 COM 과 연결), **NC**(평소 닫힘)입니다. 실습에서는 **접점에 아무것도 연결하지 않고** 소리와 LED 로만 확인합니다.

### ▶ 예제 7-4 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| 모터를 GPIO 에 직접 연결 | 돌지 않거나 **핀 손상** |
| 정방향에서 바로 역방향 | 큰 전류 → ESP32 리셋 · 드라이버 과열 → 멈춘 뒤 반대로 |
| L293D 의 GND 핀을 일부만 연결 | IC 가 뜨거워짐 — GND 4개 모두 |
| 모터 전원 GND 와 ESP32 GND 분리 | 신호 기준이 달라 제멋대로 동작 |
| 릴레이 접점에 콘센트 전원 연결 | **감전 · 화재 위험 — 이 과정에서는 절대 금지** |
| 릴레이를 PWM 으로 빠르게 | 접점이 떨리며 수명 단축 — 릴레이는 켜고 끄기만 |

#### ✏️ 빈칸 채우기 7-1

`motor_set` 을 완성합니다.

```c
void motor_set(int speed) {
    if (speed > 255) speed = 255;
    if (speed < -255) speed = ____;
    digitalWrite(PIN_IN1, speed ____ 0);
    digitalWrite(PIN_IN2, speed < 0);
    ledcWrite(PIN_EN, ____(speed));
}
```

<details><summary>🔒 정답 보기 7-1</summary>

```c
    if (speed < -255) speed = -255;
    digitalWrite(PIN_IN1, speed > 0);
    ledcWrite(PIN_EN, abs(speed));
```

speed 가 0 이면 IN1 · IN2 가 모두 LOW(자유 정지)가 됩니다.
</details>

#### ✏️ 괄호 넣기 7-2

1. 모터의 방향을 전류 방향으로 바꾸는 회로를 (　　　)라 한다.
2. IN1 = HIGH, IN2 = HIGH 이면 모터는 (　　　)가 걸린다.
3. 모터 속도는 L293D 의 (　　　) 핀에 PWM 을 준다.
4. 릴레이에서 평소에는 떨어져 있다가 켜면 COM 과 연결되는 단자는 (　　　)이다.

<details><summary>🔒 정답 보기 7-2</summary>

1. H-브리지
2. 브레이크
3. EN (EN1)
4. NO
</details>

---

# 8교시 · 스테퍼 모터 · 미니 프로젝트 · 정리 (17:00–17:50)

**학습 목표**
- 스테퍼 모터의 코일 순서(반스텝 8단계)를 배열로 구현한다
- 온도에 따라 팬과 환기구(스테퍼)를 제어하고 LCD 에 표시하는 시스템을 만든다

🎬 **D3-8 스테퍼 시퀀스** — 스텝마다 켜지는 코일과 회전자 위치, 순서를 거꾸로 하면 반대로 도는 것 확인

<img src="img/d3_L8.svg" alt="8교시 배선">

28BYJ-48 은 안의 기어 때문에 **반스텝 약 4096 스텝이 한 바퀴**입니다. 코일 4개를 정해진 순서로 켜면 한 칸씩 돌고, **순서를 거꾸로** 하면 반대로 돕니다. ULN2003 보드가 코일 전류를 대신 흘려 주며, 보드의 LED 4개로 어느 코일이 켜졌는지 보입니다.

### ▶ 예제 8-1 · 반스텝 시퀀스 — 표로 돌린다

<img src="img/d3_ex8_1.svg" alt="예제 8-1 배선">

`stepper.ino`
```c
const int COIL[4] = {16, 17, 18, 19};        // IN1 ~ IN4
const uint8_t SEQ[8] = {                     // bit0 = IN1 … bit3 = IN4
    0b0001, 0b0011, 0b0010, 0b0110,
    0b0100, 0b1100, 0b1000, 0b1001,
};
int phase = 0;

void step_once(int dir) {                    // dir = +1 또는 -1
    phase = (phase + dir + 8) % 8;           // 음수가 되지 않게 +8
    for (int i = 0; i < 4; i++)
        digitalWrite(COIL[i], (SEQ[phase] >> i) & 1);
}

void step_motor(int steps, int us_per_step) {
    int dir = steps > 0 ? 1 : -1;
    for (int s = 0; s < abs(steps); s++) {
        step_once(dir);
        delayMicroseconds(us_per_step);
    }
    for (int i = 0; i < 4; i++) digitalWrite(COIL[i], LOW);   // 멈추면 코일 끄기 (발열 방지)
}

void setup() {
    for (int i = 0; i < 4; i++) pinMode(COIL[i], OUTPUT);
}

void loop() {
    step_motor(4096, 1000);      // 한 바퀴 정방향
    delay(500);
    step_motor(-1024, 1000);     // 90° 역방향
    delay(500);
}
```

1교시 7-Segment 와 같은 **비트맵 표**입니다 — 이번에는 비트 하나가 코일 하나입니다. `(phase + dir + 8) % 8` 은 -1 이 되었을 때 7 로 돌아가게 합니다(C 의 `%` 는 음수를 음수로 남김). 스텝 간격을 약 800µs 보다 짧게 하면 모터가 따라오지 못하고 떨기만 합니다.

### ✏️ 종합 과제 8-A · 스마트 환기 시스템

<img src="img/d3_ex8_A.svg" alt="종합 과제 8-A 배선">

DHT11(GPIO4) 온도에 따라 상태를 바꿉니다.

| 상태 | 조건 | 팬 (L293D) | 환기구 (스테퍼) | LCD 2행 |
|---|---|---|---|---|
| `ST_CLOSED` | 온도 < 27°C | 정지 | 닫힘 (0°) | `VENT:CLOSE FAN:0` |
| `ST_OPEN` | 27°C 이상 | 온도에 비례 (27°C → 120, 32°C 이상 → 255) | 열림 (90°) | `VENT:OPEN FAN:xxx` |

닫힘으로 돌아가는 조건은 **26°C 미만**(히스테리시스, Day 2 5교시). 테스트할 때는 DHT11 을 손으로 감싸거나 입김을 붑니다.

`smart_vent.ino`
```c
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

const int PIN_EN = 25, PIN_IN1 = 26, PIN_IN2 = 27;
const int COIL[4] = {16, 17, 18, 19};
const uint8_t SEQ[8] = {0b0001,0b0011,0b0010,0b0110,0b0100,0b1100,0b1000,0b1001};
const int STEPS_90 = 1024;

DHT dht(4, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);

typedef enum { ST_CLOSED, ST_OPEN } vent_state_t;
vent_state_t state = ST_CLOSED;
int phase = 0;

void motor_set(int speed) {
    // TODO 1: 7교시 motor_set (이 과제는 정방향만 써도 된다)
}

void step_motor(int steps) {
    // TODO 2: 예제 8-1 의 step_once 반복 + 끝나면 코일 끄기 (1000µs 간격)
}

int fan_speed(float t) {
    // TODO 3: 27°C → 120, 32°C 이상 → 255 로 비례 (27 미만은 0)
}

void setup() {
    pinMode(PIN_IN1, OUTPUT); pinMode(PIN_IN2, OUTPUT);
    ledcAttach(PIN_EN, 20000, 8);
    for (int i = 0; i < 4; i++) pinMode(COIL[i], OUTPUT);
    dht.begin();
    lcd.init(); lcd.backlight();
}

void loop() {
    float t = dht.readTemperature();
    if (isnan(t)) { delay(2000); return; }

    // TODO 4: 상태 전이 — CLOSED 에서 27 이상이면 OPEN(스테퍼 +90°),
    //                    OPEN 에서 26 미만이면 CLOSED(스테퍼 -90°)

    int fan = (state == ST_OPEN) ? fan_speed(t) : 0;
    motor_set(fan);

    // TODO 5: LCD 1행 "T:24.0C" · 2행 "VENT:OPEN FAN:180" 고정 폭 출력

    delay(2000);
}
```

<details><summary>🔒 정답 보기 8-A (리뷰 시간에 함께 엽니다)</summary>

```c
void motor_set(int speed) {
    if (speed > 255) speed = 255;
    if (speed < 0) speed = 0;
    digitalWrite(PIN_IN1, speed > 0);
    digitalWrite(PIN_IN2, LOW);
    ledcWrite(PIN_EN, speed);
}

void step_motor(int steps) {
    int dir = steps > 0 ? 1 : -1;
    for (int s = 0; s < abs(steps); s++) {
        phase = (phase + dir + 8) % 8;
        for (int i = 0; i < 4; i++) digitalWrite(COIL[i], (SEQ[phase] >> i) & 1);
        delayMicroseconds(1000);
    }
    for (int i = 0; i < 4; i++) digitalWrite(COIL[i], LOW);
}

int fan_speed(float t) {
    if (t < 27) return 0;
    if (t >= 32) return 255;
    return 120 + (int)((t - 27) * (255 - 120) / 5);
}

    if (state == ST_CLOSED && t >= 27) { state = ST_OPEN;   step_motor(STEPS_90); }
    else if (state == ST_OPEN && t < 26) { state = ST_CLOSED; step_motor(-STEPS_90); }

    char buf[17];
    snprintf(buf, sizeof(buf), "T:%4.1fC         ", t);
    lcd.setCursor(0, 0); lcd.print(buf);
    snprintf(buf, sizeof(buf), "VENT:%-5s FAN:%3d", state == ST_OPEN ? "OPEN" : "CLOSE", fan);
    lcd.setCursor(0, 1); lcd.print(buf);
```

`%-5s` 는 왼쪽 정렬 5칸 — "OPEN" 뒤에 공백 한 칸을 채워 "CLOSE" 와 길이를 맞춥니다.
</details>

**확장 과제 (선택)**
- 4자리 7-Segment 에 온도를 `24.6` 형식(dp 사용)으로 함께 표시
- 8×8 매트릭스에 상태 아이콘(열림 ↑ / 닫힘 ↓)
- 릴레이를 "경보 출력"으로 — 35°C 이상이면 ON

**제출 체크리스트**
- [ ] 27°C 에서 열리고 26°C 아래에서 닫힌다 (그 사이에서는 상태 유지)
- [ ] 환기구가 열린 상태에서 다시 열리지 않는다 (스테퍼가 계속 돌지 않음)
- [ ] 팬 속도가 온도에 따라 변하고 LCD 에 남는 글자가 없다
- [ ] 상태는 `enum`, 동작은 함수로 나뉘어 있다

### 오늘의 여섯 문장

1. 표시 장치의 모양은 비트맵 표(데이터)로 — `(p >> i) & 1` 로 꺼내고 `|=` · `&= ~` 로 켜고 끈다 (D3-1 · D3-4)
2. 74HC595 는 DS 에 비트, SH_CP 로 밀고, ST_CP 로 한 번에 — 선 3개로 8개, 이으면 더 많이 (D3-2)
3. 여러 자리 · 여러 행은 멀티플렉싱. 갱신은 타이머 인터럽트로, 끄기 → 바꾸기 → 켜기 (D3-3)
4. I2C 는 두 선에 주소로 여러 장치. 스캐너로 주소 확인, 반환값으로 실패 확인 (D3-5 · D3-6)
5. 데이터시트의 제어 바이트는 비트 연산으로 만든다. 드라이버는 구조체 + 함수 + 오류 반환 (D3-6)
6. 모터는 드라이버를 거쳐 — 방향은 H-브리지, 속도는 PWM, 스테퍼는 코일 순서 표 (D3-7 · D3-8)

- 개념 워크시트: `anim/worksheet.html` 8교시까지 채점 후 결과 코드 제출
- 제출: `smart_vent.ino` + 동작 영상(또는 사진 2장: 닫힘 / 열림) + 워크시트 캡처
- **Day 4 예고**: FreeRTOS 태스크와 큐, LittleFS 로 로그 파일 저장, Wi-Fi 웹 서버로 센서 값을 스마트폰에서 보기, 디버깅 기법, 그리고 4일 종합 프로젝트. ESP-32U 의 **외부 안테나**를 챙겨 오세요
