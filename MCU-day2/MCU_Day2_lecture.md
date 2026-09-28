# MCU Day 2 강의 자료 — 아날로그 입력 · 센서 · 코드 모듈화

**온디바이스 AI · MCU 실습(ESP32) 2/4**

| 교시 | 주제 | 애니메이션 (anim/index.html) |
|---|---|---|
| 1 | ADC 기초 — 가변저항 | D2-1 ADC 변환기 |
| 2 | 조도 센서와 디지털 필터 | D2-2 전압 분배 · 링 버퍼 |
| 3 | 조이스틱과 서보 | D2-3 조이스틱 → 서보 |
| 4 | 온도와 습도 — LM35 · DHT11 | D2-4 LM35 · DHT11 프레임 |
| 5 | 환경 감지 — 가스 · 불꽃 · 소리 | D2-5 히스테리시스 |
| 6 | 거리와 움직임 — 초음파 · PIR | D2-6 초음파 왕복 시간 |
| 7 | 수분 센서와 코드 모듈화 | D2-7 헤더 분리 · 링크 |
| 8 | 미니 프로젝트 — 다중 센서 모니터 · 정리 | D2-8 CSV → 시리얼 플로터 |

### 이 자료 사용법

- **▶ 예제**는 교시 시작의 **배선도대로 꽂은 뒤** 스케치를 업로드하고 시리얼 모니터(115200)나 **시리얼 플로터**로 확인합니다. 주석의 `// →` 뒤가 기대 출력입니다. 센서 값은 환경마다 다르니 **변화의 방향**을 보세요.
- **✏️ 빈칸 채우기**는 `____` 를 채워 업로드합니다. **✏️ 괄호 넣기**는 ( ) 에 들어갈 말을 먼저 생각합니다.
- **🔒 정답 보기**는 강사가 신호를 준 뒤 펼칩니다.
- **⚠ 함정**은 오늘 반드시 한 번은 직접 밟아 봐야 하는 실수입니다. 오늘 함정은 대부분 **"값이 이상하다"** 로 끝납니다 — 코드보다 **전압과 배선**을 먼저 의심하세요.
- Day 1 의 `button_t` 처럼 오늘은 **센서도 구조체**로 다루고, C 과정 Day 2 의 포인터 · 배열 · 링 버퍼, Day 1 의 헤더 분리를 실제 센서에 적용합니다.

---

## ⚙️ 준비 — 오늘의 배선 규칙

Day 1 과 같이 **ESP32 는 브레드보드 옆에 두고 M-F 점퍼선**으로 연결합니다. 오늘은 전원 레일을 세 가지로 씁니다.

| 레일 | 연결 | 쓰는 부품 |
|---|---|---|
| **위 빨강 (+)** | ESP32 **3V3** | 가변저항, 조도, 조이스틱, DHT11, 불꽃 · 소리 모듈 |
| **위 · 아래 파랑 (−)** | ESP32 **GND** (두 개의 GND 핀) | 모든 부품 |
| **아래 빨강 (+)** | ESP32 **5V** — 필요한 교시에만 | 서보, LM35, MQ-2, HC-SR04, PIR |

> **5V 레일 규칙** — 5V 는 **부품의 전원(VCC)** 에만 씁니다. 5V 로 동작하는 부품의 **출력 신호**가 5V 까지 올라가면(MQ-2 AO, HC-SR04 Echo) 반드시 **저항 두 개로 분압**해서 GPIO 에 넣습니다.

**오늘 쓰는 ADC 핀은 ADC1 여섯 개뿐입니다**: 32 · 33 · 34 · 35 · 36(VP) · 39(VN). Wi-Fi 를 켜도 동작하고, 34~39 는 입력 전용이라 센서 전용으로 딱 맞습니다.

| GPIO | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|
| 34 | 가변저항 | | | | 불꽃 | | 토양 | 가변저항 |
| 35 | | 조도 | | | 소리 | | 수위 | 조도 |
| 32 | | | | | MQ-2 | | | |
| 33 | | | | LM35 | | | | LM35 |
| 36 · 39 | | | 조이스틱 X · Y | | | | | |
| 디지털 | | LED 25 | SW 4 · 서보 18 | DHT11 27 | 부저 19 | Trig 17 · Echo 16 · PIR 13 · LED 25 | 전원 14 · 23 | DHT11 27 |

교시마다 **그 교시의 배선도**가 있습니다. 앞 교시 부품은 빼도 되고 그대로 둬도 됩니다(열이 겹치면 뺍니다). **센서 모듈의 핀 순서는 제품마다 다르므로**, 꽂기 전에 모듈 기판에 인쇄된 글자(S/V/G, AO/VCC/GND, OUT 등)를 **반드시 읽고** 그림의 글자와 맞춥니다.

### 🛠 따라하기 · DHT 라이브러리 설치 (4교시 전)

1. Arduino IDE 왼쪽 세로 막대의 **라이브러리 매니저**(세 번째 아이콘, 책 모양)를 누르거나 **스케치 → 라이브러리 포함하기 → 라이브러리 관리** 를 엽니다.
2. 검색 칸에 `DHT sensor library` 를 입력합니다.
3. **DHT sensor library by Adafruit** 의 **설치** 를 누릅니다.
4. "필요한 라이브러리도 함께 설치할까요?" 창이 뜨면 **모두 설치(Install All)** 를 누릅니다 — `Adafruit Unified Sensor` 가 함께 설치됩니다.
5. 출력 창에 `Installed DHT sensor library` 가 나오면 완료입니다.

| 증상 | 조치 |
|---|---|
| `DHT.h: No such file or directory` | 3번을 하지 않았거나 다른 이름의 라이브러리를 설치함 → Adafruit 것으로 |
| `Adafruit_Sensor.h: No such file` | 4번에서 "모두 설치"를 누르지 않음 → `Adafruit Unified Sensor` 를 따로 설치 |

오늘 나머지 센서(가변저항 · 조도 · LM35 · MQ-2 · 초음파 · PIR · 서보)는 **라이브러리 없이** 직접 구현합니다.

---

# 1교시 · ADC 기초 — 가변저항 (09:00–09:50)

**학습 목표**
- ADC 가 전압을 0~4095 정수로 바꾸는 원리(12비트, 감쇠, 비선형)를 설명한다
- `analogRead` 와 `analogReadMilliVolts` 의 차이를 안다
- 배열과 포인터 인자로 평균 · 최소 · 최대를 구하는 함수를 만든다

🎬 **D2-1 ADC 변환기** — 전압 슬라이더를 움직여 raw 값과 mV 값, 양 끝의 비선형 구간 확인

<img src="img/d2_L1.svg" alt="1교시 배선">

가변저항은 다리가 3개입니다. **양 끝 다리는 3V3 와 GND**, **가운데 다리(와이퍼)** 는 손잡이를 돌리면 0V~3.3V 사이 전압이 나옵니다. 양 끝을 바꿔 꽂으면 돌리는 방향만 반대가 될 뿐 고장나지 않습니다.

### ▶ 예제 1-1 · analogRead — 전압을 숫자로

<img src="img/d2_ex1_1.svg" alt="예제 1-1 배선">

`adc_raw.ino`
```c
const int PIN_POT = 34;

void setup() {
    Serial.begin(115200);
}

void loop() {
    int raw = analogRead(PIN_POT);          // 0 ~ 4095 (12비트)
    Serial.printf("raw = %4d\n", raw);
    delay(200);
}
// → 손잡이를 끝까지 왼쪽: raw = 0 ~ 수십 / 끝까지 오른쪽: raw = 4095
```

ESP32 ADC 는 **12비트** 라서 0~4095 (2¹² = 4096 단계) 를 돌려줍니다. Arduino Uno 는 10비트(0~1023) 였습니다. `pinMode` 는 필요 없습니다.

### ▶ 예제 1-2 · raw 와 mV — 보정된 전압

<img src="img/d2_ex1_2.svg" alt="예제 1-2 배선">

`adc_mv.ino`
```c
const int PIN_POT = 34;

void setup() {
    Serial.begin(115200);
}

void loop() {
    int raw = analogRead(PIN_POT);
    int mv  = analogReadMilliVolts(PIN_POT);          // 칩마다 보정된 mV
    float naive = raw * 3300.0f / 4095;               // 단순 비례 계산
    Serial.printf("raw=%4d  mv=%4d  naive=%6.1f\n", raw, mv, naive);
    delay(300);
}
// → raw=2048  mv=1680  naive=1650.6   (예시 — 칩마다 다름)
```

| 구분 | 의미 |
|---|---|
| `analogRead` | ADC 가 낸 숫자 그대로. 칩마다 조금씩 다르고 **양 끝(0V 근처, 3.1V 이상)에서 비선형** |
| `analogReadMilliVolts` | 칩 안에 저장된 **보정값(eFuse)** 으로 계산한 mV. 전압이 필요하면 이것을 쓴다 |
| 감쇠(attenuation) | 기본 **11dB** — 약 0.15V ~ 3.1V 를 잴 수 있다. 그 밖은 0 또는 4095 로 붙는다 |

### ▶ 예제 1-3 · 여러 번 읽어 평균 — 포인터 인자

<img src="img/d2_ex1_3.svg" alt="예제 1-3 배선">

`adc_avg.ino`
```c
const int PIN_POT = 34;

// n 번 읽어 평균을 돌려주고, 최소 · 최대는 포인터로 돌려준다
int read_avg(int pin, int n, int *min_out, int *max_out) {
    long sum = 0;
    int mn = 4095, mx = 0;
    for (int i = 0; i < n; i++) {
        int v = analogRead(pin);
        sum += v;
        if (v < mn) mn = v;
        if (v > mx) mx = v;
    }
    *min_out = mn;
    *max_out = mx;
    return sum / n;
}

void setup() {
    Serial.begin(115200);
}

void loop() {
    int mn, mx;
    int avg = read_avg(PIN_POT, 32, &mn, &mx);
    Serial.printf("avg=%4d  min=%4d  max=%4d  noise=%d\n", avg, mn, mx, mx - mn);
    delay(300);
}
// → avg=2040  min=2021  max=2063  noise=42   (손잡이를 가만히 둬도 값이 흔들린다)
```

**반환값은 하나**라서, 여러 결과는 **주소를 받아 거기에 써 줍니다** (C 과정 Day 2 의 `max_to(a, b, &out)` 모양). 손을 대지 않아도 `noise` 가 수십 나오는 것이 정상입니다 — 2교시에 필터로 줄입니다.

### ▶ 예제 1-4 · ⚠ 함정 — ADC2 핀과 Wi-Fi, 그리고 5V

| 한 일 | 결과 |
|---|---|
| 가변저항 가운데 다리를 GPIO **25** (ADC2) 에 연결 | 지금은 읽히지만 **Day 4 에서 Wi-Fi 를 켜면 값이 안 나옴** — ADC 는 처음부터 **ADC1(32~39)** 에 |
| 가변저항 한쪽 끝을 **5V** 에 연결 | 3.1V 이상 구간에서 계속 4095 — 그리고 핀에 3.3V 초과 전압이 들어가 **손상 위험** |
| 가운데 다리가 아닌 **끝 다리**를 GPIO 에 연결 | 돌려도 값이 0 이나 4095 에서 안 변함 |

#### ✏️ 빈칸 채우기 1-1

배열에 모은 값의 최소 · 최대 · 평균을 한 번에 구합니다.

```c
const int PIN_POT = 34;
const int N = 16;
int samples[N];

void stats(const int *a, int n, int *mn, int *mx, float *avg) {
    long sum = 0;
    *mn = a[0];
    *mx = a[0];
    for (int i = 0; i < n; i++) {
        if (a[i] < *mn) *mn = a[i];
        if (a[i] > ____) *mx = a[i];
        sum += a[i];
    }
    ____ = (float)sum / n;
}

void setup() { Serial.begin(115200); }

void loop() {
    for (int i = 0; i < N; i++) samples[i] = analogRead(____);
    int mn, mx; float avg;
    stats(samples, N, &mn, &mx, ____);
    Serial.printf("min=%d max=%d avg=%.1f\n", mn, mx, avg);
    delay(500);
}
```

<details><summary>🔒 정답 보기 1-1</summary>

```c
        if (a[i] > *mx) *mx = a[i];
    *avg = (float)sum / n;
    for (int i = 0; i < N; i++) samples[i] = analogRead(PIN_POT);
    stats(samples, N, &mn, &mx, &avg);
```

배열은 함수에 넘기면 포인터(`const int *a`)가 되므로 길이 `n` 을 따로 넘깁니다. `const` 는 "읽기만 한다"는 약속입니다.
</details>

#### ✏️ 괄호 넣기 1-2

1. ESP32 ADC 는 (　　　)비트라서 값의 범위는 0 ~ (　　　) 이다.
2. 칩마다 보정된 전압을 mV 로 주는 함수는 (　　　) 이다.
3. Wi-Fi 를 켜도 쓸 수 있는 ADC 는 (`ADC1` / `ADC2`) 이며 GPIO 32 ~ (　　　) 이다.
4. 가변저항의 (　　　) 다리를 GPIO 에 연결한다.

<details><summary>🔒 정답 보기 1-2</summary>

1. 12 / 4095
2. analogReadMilliVolts
3. ADC1 / 39
4. 가운데
</details>

---

# 2교시 · 조도 센서와 디지털 필터 (10:00–10:50)

**학습 목표**
- 전압 분배(분압) 회로로 저항 변화를 전압 변화로 바꾼다
- 링 버퍼로 이동평균 필터를 구현한다
- 밝기에 따라 LED 를 PWM 으로 켜는 자동 조명을 만든다

🎬 **D2-2 전압 분배 · 링 버퍼** — 빛 세기를 바꿔 분압 전압 확인, 값이 링 버퍼에 들어가며 평균이 바뀌는 과정

<img src="img/d2_L2.svg" alt="2교시 배선">

### ▶ 예제 2-1 · 전압 분배 — 저항을 전압으로

<img src="img/d2_ex2_1.svg" alt="예제 2-1 배선">

포토레지스터(CdS)는 **밝으면 저항이 작아지고, 어두우면 커지는** 부품입니다. ADC 는 저항을 직접 못 재므로 **10kΩ 과 직렬로 연결해 가운데 전압**을 읽습니다.

**Vout = 3.3V × R_10k ÷ (R_photo + R_10k)**

| 상황 | R_photo (대략) | Vout | raw |
|---|---|---|---|
| 밝음 (손전등) | 1kΩ | 3.0V | 약 3700 |
| 실내 | 10kΩ | 1.65V | 약 2000 |
| 손으로 가림 | 100kΩ | 0.3V | 약 300 |

`light.ino`
```c
const int PIN_LIGHT = 35;

void setup() { Serial.begin(115200); }

void loop() {
    int raw = analogRead(PIN_LIGHT);
    Serial.printf("light:%d\n", raw);         // 시리얼 플로터용 "이름:값"
    delay(50);
}
// → 손으로 가리면 값이 작아지고, 밝히면 커진다
```

**도구 → 시리얼 플로터** 를 열면 그래프로 보입니다. `이름:값` 형식으로 출력하면 이름이 범례로 나옵니다.

### ▶ 예제 2-2 · 링 버퍼 이동평균 — 흔들림 줄이기

<img src="img/d2_ex2_2.svg" alt="예제 2-2 배선">

`ring_filter.ino`
```c
#define RN 8
typedef struct {
    int buf[RN];     // 최근 RN 개 값
    int head;        // 다음에 쓸 위치
    int count;       // 지금까지 들어온 개수 (최대 RN)
    long sum;        // buf 합계 — 매번 다시 더하지 않는다
} ring_t;

int ring_push(ring_t *r, int v) {
    if (r->count == RN) r->sum -= r->buf[r->head];   // 가장 오래된 값 빼기
    else r->count++;
    r->buf[r->head] = v;
    r->sum += v;
    r->head = (r->head + 1) % RN;                     // 끝에 가면 0 으로
    return r->sum / r->count;
}

ring_t light = {0};
const int PIN_LIGHT = 35;

void setup() { Serial.begin(115200); }

void loop() {
    int raw = analogRead(PIN_LIGHT);
    int avg = ring_push(&light, raw);
    Serial.printf("raw:%d,avg:%d\n", raw, avg);      // 두 줄이 함께 그려진다
    delay(20);
}
```

`head` 가 배열 끝에 닿으면 `% RN` 으로 **처음으로 돌아가** 가장 오래된 값을 덮어씁니다 — 그래서 "링(고리)" 버퍼입니다. 합계를 들고 다니므로 RN 이 커져도 계산은 **더하기 한 번, 빼기 한 번**입니다.

### ▶ 예제 2-3 · 자동 조명 — 어두울수록 밝게

<img src="img/d2_ex2_3.svg" alt="예제 2-3 배선">

`auto_light.ino`
```c
// 예제 2-2 에서 #define RN ~ ring_push 함수 끝까지만 위에 붙여 넣는다
const int PIN_LIGHT = 35, PIN_LED = 25;
ring_t light = {0};

void setup() {
    Serial.begin(115200);
    ledcAttach(PIN_LED, 5000, 8);
}

void loop() {
    int avg = ring_push(&light, analogRead(PIN_LIGHT));
    int duty = 255 - avg * 255 / 4095;          // 어두울수록(avg 작을수록) 크게
    if (duty < 20) duty = 0;                    // 충분히 밝으면 끈다
    ledcWrite(PIN_LED, duty);
    Serial.printf("avg:%d,duty:%d\n", avg, duty);
    delay(20);
}
```

Day 1 7교시의 `ledcAttach` / `ledcWrite` 가 그대로 쓰입니다. 조도 센서를 손으로 가리면 LED 가 점점 밝아집니다.

### ▶ 예제 2-4 · ⚠ 함정 — 10kΩ 을 빼먹으면

| 한 일 | 결과 |
|---|---|
| 포토레지스터만 3V3 ↔ GPIO 에 연결 (10kΩ 없음) | GPIO 가 3V3 에 거의 직결 → 밝기와 상관없이 **4095 근처** |
| 10kΩ 대신 220Ω | 가운데 전압이 거의 0 → 항상 **작은 값**. 색띠 확인: 10kΩ = 갈색 검정 주황 |
| LED 를 조도 센서 바로 옆에 둠 | LED 빛이 센서로 들어가 **깜빡이며 진동** — 켜지면 밝다고 판단해 꺼지고, 꺼지면 다시 켜진다 |

#### ✏️ 빈칸 채우기 2-1

`ring_push` 를 완성합니다.

```c
#define RN 8
typedef struct { int buf[RN]; int head; int count; long sum; } ring_t;

int ring_push(ring_t *r, int v) {
    if (r->count == RN) r->sum -= r->buf[____];
    else r->count++;
    r->buf[r->head] = v;
    r->sum += ____;
    r->head = (r->head + 1) % ____;
    return r->sum / r->____;
}
```

<details><summary>🔒 정답 보기 2-1</summary>

```c
    if (r->count == RN) r->sum -= r->buf[r->head];
    r->sum += v;
    r->head = (r->head + 1) % RN;
    return r->sum / r->count;
```

버퍼가 가득 찼을 때 `head` 자리는 **가장 오래된 값**입니다. 그 값을 합계에서 빼고 새 값으로 덮어씁니다. 가득 차기 전에는 `count` 로 나눠야 처음 몇 개도 올바른 평균이 됩니다.
</details>

#### ✏️ 괄호 넣기 2-2

1. 포토레지스터는 밝을수록 저항이 (`커진다` / `작아진다`).
2. 저항 변화를 전압으로 바꾸려고 10kΩ 과 직렬로 연결한 회로를 (　　　) 회로라 한다.
3. 링 버퍼의 다음 위치는 `(head + 1) (　　　) RN` 으로 계산한다.
4. 시리얼 플로터에서 범례가 나오게 하려면 `(　　　):값` 형식으로 출력한다.

<details><summary>🔒 정답 보기 2-2</summary>

1. 작아진다
2. 전압 분배 (분압)
3. %
4. 이름
</details>

---

# 3교시 · 조이스틱과 서보 (11:00–11:50)

**학습 목표**
- 2축 조이스틱의 X · Y 를 ADC 로, 버튼을 디지털로 읽는다
- `map` 을 직접 구현하고 데드존을 적용한다
- LEDC 50Hz PWM 의 펄스 폭으로 서보 각도를 제어한다

🎬 **D2-3 조이스틱 → 서보** — 조이스틱을 끌어 raw → 각도 → 펄스 폭 변환과 서보 회전 확인

<img src="img/d2_L3.svg" alt="3교시 배선">

> **조이스틱 전원은 3V3** — 모듈에 `+5V` 라고 인쇄되어 있어도 **위 빨강(3V3) 레일**에 꽂습니다. 5V 를 주면 X · Y 출력이 3.3V 를 넘어 ADC 가 4095 에 붙고 핀이 손상될 수 있습니다. **서보는 반대로 5V** (아래 레일) 입니다.

### ▶ 예제 3-1 · 조이스틱 읽기

<img src="img/d2_ex3_1.svg" alt="예제 3-1 배선">

`joystick.ino`
```c
const int PIN_X = 36, PIN_Y = 39, PIN_SW = 4;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_SW, INPUT_PULLUP);          // 누르면 LOW (Day 1 5교시)
}

void loop() {
    int x = analogRead(PIN_X);
    int y = analogRead(PIN_Y);
    int sw = digitalRead(PIN_SW) == LOW;
    Serial.printf("x:%d,y:%d,sw:%d\n", x, y, sw * 4095);
    delay(50);
}
// → 가운데: x, y 약 1800~2000 / 끝까지 밀면 0 또는 4095 / 누르면 sw:4095
```

가운데 값이 정확히 2048 이 아니고 제품마다 다릅니다. **처음에 가운데 값을 한 번 재서 저장**해 두는 것이 좋습니다(빈칸 3-1).

### ▶ 예제 3-2 · 서보 — 펄스 폭이 각도다

<img src="img/d2_ex3_2.svg" alt="예제 3-2 배선">

SG90 서보는 **20ms(50Hz) 마다 한 번** 오는 펄스의 **HIGH 길이**로 각도를 정합니다: 약 **0.5ms = 0°, 1.5ms = 90°, 2.5ms = 180°**. Day 1 의 LEDC 로 직접 만듭니다 — 라이브러리가 필요 없습니다.

`servo.ino`
```c
const int PIN_SERVO = 18;
const int BITS = 14;                         // 14비트 → 0 ~ 16383
const int MAX_DUTY = (1 << BITS) - 1;

void servo_write_us(int us) {                // 펄스 폭(µs) → 듀티
    ledcWrite(PIN_SERVO, (long)us * MAX_DUTY / 20000);
}

void servo_write_deg(int deg) {
    if (deg < 0) deg = 0;
    if (deg > 180) deg = 180;
    servo_write_us(500 + deg * 2000 / 180);  // 0° → 500µs, 180° → 2500µs
}

void setup() {
    ledcAttach(PIN_SERVO, 50, BITS);          // 50Hz = 20ms 주기
}

void loop() {
    servo_write_deg(0);   delay(1000);
    servo_write_deg(90);  delay(1000);
    servo_write_deg(180); delay(1000);
}
```

1.5ms 펄스는 20ms 의 7.5% 입니다. 14비트(16383 단계)로 나누면 한 단계가 약 1.2µs 라서 각도를 부드럽게 줄 수 있습니다. 8비트면 한 단계가 78µs 라 너무 거칩니다.

### ▶ 예제 3-3 · 조이스틱으로 서보 조종 — map 직접 만들기

<img src="img/d2_ex3_3.svg" alt="예제 3-3 배선">

`joy_servo.ino`
```c
// 예제 3-2 의 PIN_SERVO ~ servo_write_deg 끝까지(setup 전까지)를 위에 붙여 넣는다
const int PIN_X = 36;

long my_map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
    Serial.begin(115200);
    ledcAttach(PIN_SERVO, 50, BITS);
}

void loop() {
    int x = analogRead(PIN_X);
    int deg = my_map(x, 0, 4095, 0, 180);
    servo_write_deg(deg);
    Serial.printf("x:%d,deg:%d\n", x, deg);
    delay(20);
}
```

`my_map` 은 "입력 범위의 몇 % 위치인가"를 출력 범위에 옮기는 비례식입니다. **곱하기를 먼저, 나누기를 나중에** 해야 정수 나눗셈으로 0 이 되어 버리지 않습니다.

### ▶ 예제 3-4 · ⚠ 함정 — 서보를 3V3 에, 서보가 떨린다

| 한 일 | 결과 |
|---|---|
| 서보 빨강 선을 **3V3** 에 | 힘이 약해 떨리거나 안 움직임. 움직일 때 전류가 커서 **ESP32 가 리셋**(brown-out)될 수 있음 |
| 서보를 5V 에 꽂았는데 **GND 를 공통으로 안 함** | 신호의 기준이 달라 **무작위로 떨림** — 서보 갈색 선은 반드시 ESP32 GND 와 같은 레일 |
| `ledcAttach(18, 5000, 8)` 로 서보 제어 | 주기가 0.2ms 라 서보가 펄스를 해석하지 못함 → **50Hz** |
| 조이스틱 가운데에서도 서보가 조금씩 움찔 | 가운데 값의 잡음 때문 → **데드존** (빈칸 3-1) |

#### ✏️ 빈칸 채우기 3-1

시작할 때 가운데 값을 저장하고, 가운데 ±150 안에서는 90° 로 고정합니다(데드존).

```c
// 예제 3-3 의 PIN_SERVO ~ my_map 까지를 위에 붙여 넣는다
const int PIN_X = 36;
int center;

void setup() {
    ledcAttach(PIN_SERVO, ____, 14);
    center = analogRead(____);            // 손대지 않은 상태의 값
}

void loop() {
    int x = analogRead(PIN_X);
    int deg;
    if (abs(x - center) < ____) deg = 90;
    else deg = my_map(x, 0, 4095, 0, 180);
    servo_write_deg(____);
    delay(20);
}
```

<details><summary>🔒 정답 보기 3-1</summary>

```c
    ledcAttach(PIN_SERVO, 50, 14);
    center = analogRead(PIN_X);
    if (abs(x - center) < 150) deg = 90;
    servo_write_deg(deg);
```

데드존은 "이 정도 흔들림은 무시한다"는 구간입니다. 5교시 히스테리시스와 같은 발상입니다.
</details>

#### ✏️ 괄호 넣기 3-2

1. 서보는 (　　　)Hz, 즉 20ms 마다 오는 펄스의 HIGH 길이로 각도를 정한다.
2. SG90 에서 약 1.5ms 펄스는 (　　　)° 이다.
3. 조이스틱 모듈의 전원은 (`3V3` / `5V`) 레일에, 서보 전원은 (`3V3` / `5V`) 레일에 연결한다.
4. 가운데 근처의 작은 흔들림을 무시하는 구간을 (　　　)이라 한다.

<details><summary>🔒 정답 보기 3-2</summary>

1. 50
2. 90
3. 3V3 / 5V
4. 데드존 (dead zone)
</details>

---

# 4교시 · 온도와 습도 — LM35 · DHT11 (13:00–13:50)

**학습 목표**
- 아날로그 온도 센서(LM35)의 mV 를 °C 로 바꾼다
- 디지털 센서(DHT11)를 라이브러리로 읽고 실패(NaN)를 처리한다
- 측정값과 시각을 구조체로 묶어 설계한다

🎬 **D2-4 LM35 · DHT11 프레임** — 온도 슬라이더로 LM35 mV 확인, DHT11 이 보내는 40비트 데이터 해부

<img src="img/d2_L4.svg" alt="4교시 배선">

> **LM35 방향** — 글자가 쓰인 **평평한 면을 나를 향하게** 두면 왼쪽부터 `+Vs · Vout · GND` 입니다. 거꾸로 꽂으면 **뜨거워집니다** — 만져서 뜨거우면 즉시 USB 를 뽑으세요. LM35 는 **4V 이상**이 필요하므로 **5V 레일**에 연결하고, 출력은 10mV/°C 라 25°C 에 0.25V 뿐이어서 분압 없이 GPIO 에 연결해도 안전합니다.

### ▶ 예제 4-1 · LM35 — 10mV 가 1°C

<img src="img/d2_ex4_1.svg" alt="예제 4-1 배선">

`lm35.ino`
```c
const int PIN_LM35 = 33;

void setup() { Serial.begin(115200); }

void loop() {
    long sum = 0;
    for (int i = 0; i < 16; i++) sum += analogReadMilliVolts(PIN_LM35);
    float mv = sum / 16.0f;
    float temp_c = mv / 10.0f;               // 10mV = 1°C
    Serial.printf("mv:%.0f,temp:%.1f\n", mv, temp_c);
    delay(500);
}
// → mv:245,temp:24.5   손가락으로 잡으면 천천히 오른다
```

250mV 는 ADC 감쇠 11dB 의 아래쪽 비선형 구간 근처라 `analogRead` 비례 계산은 오차가 큽니다. **반드시 `analogReadMilliVolts`** 를 쓰고 여러 번 평균합니다.

### ▶ 예제 4-2 · DHT11 — 라이브러리로 읽기

<img src="img/d2_ex4_2.svg" alt="예제 4-2 배선">

준비의 **따라하기 · DHT 라이브러리 설치** 를 먼저 합니다.

`dht11.ino`
```c
#include <DHT.h>

const int PIN_DHT = 27;
DHT dht(PIN_DHT, DHT11);

void setup() {
    Serial.begin(115200);
    dht.begin();
}

void loop() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();         // °C
    if (isnan(h) || isnan(t)) {
        Serial.println("DHT 읽기 실패");
    } else {
        Serial.printf("temp:%.1f,humi:%.1f\n", t, h);
    }
    delay(2000);                             // DHT11 은 1초에 한 번 이하
}
// → temp:25.0,humi:48.0   입김을 불면 습도가 오른다
```

DHT11 은 선 하나로 **40비트(습도 16 + 온도 16 + 체크섬 8)** 를 정해진 타이밍으로 보냅니다. 체크섬이 틀리거나 응답이 없으면 라이브러리가 `NaN`(Not a Number) 을 돌려줍니다. `NaN` 은 `==` 로 비교할 수 없으므로 **`isnan()`** 으로 검사합니다.

### ▶ 예제 4-3 · 측정값 구조체 — 두 센서 비교

<img src="img/d2_ex4_3.svg" alt="예제 4-3 배선">

`temp_compare.ino`
```c
#include <DHT.h>

typedef struct {
    float temp_c;
    float humi;          // 습도가 없는 센서는 NAN
    uint32_t t_ms;       // 잰 시각
    bool ok;
} reading_t;

DHT dht(27, DHT11);

reading_t read_lm35(void) {
    reading_t r;
    long sum = 0;
    for (int i = 0; i < 16; i++) sum += analogReadMilliVolts(33);
    r.temp_c = sum / 16.0f / 10.0f;
    r.humi = NAN;
    r.t_ms = millis();
    r.ok = true;
    return r;
}

reading_t read_dht(void) {
    reading_t r;
    r.temp_c = dht.readTemperature();
    r.humi = dht.readHumidity();
    r.t_ms = millis();
    r.ok = !isnan(r.temp_c) && !isnan(r.humi);
    return r;
}

void setup() { Serial.begin(115200); dht.begin(); }

void loop() {
    reading_t a = read_lm35();
    reading_t b = read_dht();
    if (b.ok) Serial.printf("lm35:%.1f,dht:%.1f,diff:%.1f\n", a.temp_c, b.temp_c, a.temp_c - b.temp_c);
    delay(2000);
}
```

두 센서가 1~2°C 다른 것은 정상입니다(DHT11 정확도 ±2°C, 분해능 1°C). 구조체로 묶으면 **"값 + 언제 + 유효한가"** 를 한 번에 넘길 수 있습니다 — 8교시에 배열로 확장합니다.

### ▶ 예제 4-4 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| LM35 를 거꾸로(평평한 면이 뒤) | 센서가 뜨거워지고 값이 엉뚱함 → **즉시 USB 분리** |
| LM35 를 3V3 에 연결 | 동작 전압 부족 → 값이 불안정하거나 0 |
| DHT11 을 `delay(100)` 마다 읽음 | 절반 이상 **읽기 실패(NaN)** — 최소 1~2초 간격 |
| `if (t == NAN)` 으로 실패 검사 | NaN 은 자기 자신과도 같지 않아 **항상 거짓** → `isnan(t)` |

#### ✏️ 빈칸 채우기 4-1

LM35 를 읽어 구조체를 채우는 함수입니다.

```c
typedef struct { float temp_c; float humi; uint32_t t_ms; bool ok; } reading_t;

reading_t read_lm35(int pin) {
    reading_t r;
    long sum = 0;
    for (int i = 0; i < 16; i++) sum += ____(pin);
    r.temp_c = sum / 16.0f / ____;
    r.humi = NAN;
    r.t_ms = ____();
    r.ok = (r.temp_c > -10 && r.temp_c < 100);
    return ____;
}
```

<details><summary>🔒 정답 보기 4-1</summary>

```c
    for (int i = 0; i < 16; i++) sum += analogReadMilliVolts(pin);
    r.temp_c = sum / 16.0f / 10.0f;
    r.t_ms = millis();
    return r;
```

구조체는 **값으로 통째로 반환**할 수 있습니다(C 과정 Day 3). 16바이트 정도는 복사해도 부담이 없습니다.
</details>

#### ✏️ 괄호 넣기 4-2

1. LM35 의 출력은 1°C 당 (　　　)mV 이다.
2. LM35 는 (`3V3` / `5V`) 레일에 연결한다.
3. DHT11 읽기가 실패하면 라이브러리는 (　　　)을 돌려주며, 이는 `(　　　)()` 함수로 검사한다.
4. DHT11 은 (　　　)초에 한 번 이하로 읽는다.

<details><summary>🔒 정답 보기 4-2</summary>

1. 10
2. 5V
3. NaN / isnan
4. 1 (1~2)
</details>

---

# 5교시 · 환경 감지 — 가스 · 불꽃 · 소리 (14:00–14:50)

**학습 목표**
- 5V 출력 센서를 분압해 안전하게 읽는다
- 히스테리시스로 경보가 떨리지 않게 한다
- 여러 센서의 경보를 하나의 출력(부저)으로 모은다

🎬 **D2-5 히스테리시스** — 잡음 섞인 센서 값에 임계값 하나 / 두 개를 적용해 경보가 몇 번 바뀌는지 비교

<img src="img/d2_L5.svg" alt="5교시 배선">

> **MQ-2 예열** — MQ-2 는 안의 히터가 데워져야 값이 안정됩니다. 전원을 넣고 **최소 1~2분** 기다리고, 처음 쓰는 센서는 수십 분 동안 값이 계속 내려갑니다. 히터 때문에 **모듈이 따뜻해지는 것은 정상**입니다. 테스트는 **라이터 가스를 불을 붙이지 않고** 살짝 쐬는 정도로만 합니다.
>
> **불꽃 · 소리 모듈**은 제품에 따라 핀이 `S V G`, `AO VCC GND`, 또는 `DO` 까지 4핀일 수 있습니다. **아날로그 출력(S 또는 AO)** 을 GPIO 에 연결합니다.

### ▶ 예제 5-1 · MQ-2 — 5V 출력은 반으로

<img src="img/d2_ex5_1.svg" alt="예제 5-1 배선">

MQ-2 의 AO 는 가스가 많을수록 올라가 **5V 까지** 나옵니다. 10kΩ 두 개로 **절반(최대 2.5V)** 으로 만들어 GPIO32 에 넣습니다.

**V_gpio = V_AO × 10k ÷ (10k + 10k) = V_AO ÷ 2**

`mq2.ino`
```c
const int PIN_GAS = 32;

void setup() { Serial.begin(115200); }

void loop() {
    int mv = analogReadMilliVolts(PIN_GAS);
    int ao_mv = mv * 2;                        // 분압 전 실제 AO 전압
    Serial.printf("gas_mv:%d\n", ao_mv);
    delay(200);
}
// → 깨끗한 공기: 수백 mV / 라이터 가스를 살짝: 2000mV 이상으로 치솟았다가 천천히 복귀
```

### ▶ 예제 5-2 · 불꽃 · 소리 — 같은 모양, 다른 센서

<img src="img/d2_ex5_2.svg" alt="예제 5-2 배선">

`flame_sound.ino`
```c
const int PIN_FLAME = 34, PIN_SOUND = 35;

void setup() { Serial.begin(115200); }

void loop() {
    int flame = analogRead(PIN_FLAME);
    int sound_max = 0;
    for (int i = 0; i < 200; i++) {            // 소리는 빠르게 흔들리므로 최대값을 본다
        int v = analogRead(PIN_SOUND);
        if (v > sound_max) sound_max = v;
    }
    Serial.printf("flame:%d,sound:%d\n", flame, sound_max);
    delay(50);
}
```

불꽃 센서는 제품에 따라 **불꽃이 가까울수록 값이 커지거나 작아집니다** — 라이터(또는 리모컨 적외선)를 비춰 **어느 방향으로 변하는지 먼저 확인**합니다. 소리는 파동이라 한 번 읽은 값은 의미가 적고, 짧은 시간의 **최대값(또는 최대−최소)** 을 봅니다. 손뼉을 쳐 보세요.

### ▶ 예제 5-3 · 히스테리시스 — 경보가 떨리지 않게

<img src="img/d2_ex5_3.svg" alt="예제 5-3 배선">

`gas_alarm.ino`
```c
const int PIN_GAS = 32, PIN_BUZ = 19;
const int ON_MV  = 1500;      // 이 위로 올라가면 경보 ON
const int OFF_MV = 1200;      // 이 아래로 내려가야 경보 OFF
bool alarm = false;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BUZ, OUTPUT);
}

void loop() {
    int ao = analogReadMilliVolts(PIN_GAS) * 2;
    if (!alarm && ao > ON_MV)       alarm = true;
    else if (alarm && ao < OFF_MV)  alarm = false;
    digitalWrite(PIN_BUZ, alarm);
    Serial.printf("gas:%d,on:%d,off:%d,alarm:%d\n", ao, ON_MV, OFF_MV, alarm * 2000);
    delay(100);
}
```

임계값이 **하나**면 값이 1500 근처에서 흔들릴 때 경보가 **켜졌다 꺼졌다를 반복**합니다. 켜는 선과 끄는 선을 **다르게** 두면(1500 / 1200) 그 사이에서는 **이전 상태를 유지**합니다. 에어컨 온도 조절, 냉장고가 모두 이 방식입니다.

### ▶ 예제 5-4 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| MQ-2 AO 를 **분압 없이** GPIO 에 | 가스가 많을 때 5V 가 핀에 → **손상 위험** |
| MQ-2 를 3V3 에 연결 | 히터가 충분히 데워지지 않아 값이 거의 안 변함 |
| 켜자마자 임계값을 정함 | 예열 중이라 값이 계속 내려가 → **1~2분 뒤** 깨끗한 공기 값을 보고 정한다 |
| 소리를 `analogRead` 한 번으로 판단 | 파동의 아무 순간이나 잡혀 값이 제멋대로 |

#### ✏️ 빈칸 채우기 5-1

세 센서 중 하나라도 경보면 부저를 울립니다. 경보 판단은 구조체와 함수 하나로 처리합니다.

```c
typedef struct {
    int pin;
    int on_th, off_th;
    bool alarm;
} alarm_t;

bool update_alarm(alarm_t *a, int value) {
    if (!a->alarm && value > a->____)      a->alarm = true;
    else if (a->alarm && value < a->____)  a->alarm = false;
    return a->alarm;
}

alarm_t gas   = {32, 1500, 1200, false};
alarm_t flame = {34, 3000, 2500, false};
alarm_t sound = {35, 3500, 3000, false};

void loop() {
    bool any = false;
    any |= update_alarm(&gas, analogReadMilliVolts(gas.pin) * 2);
    any |= update_alarm(____, analogRead(flame.pin));
    any |= update_alarm(&sound, analogRead(sound.pin));
    digitalWrite(19, ____);
    delay(100);
}
```

<details><summary>🔒 정답 보기 5-1</summary>

```c
    if (!a->alarm && value > a->on_th)      a->alarm = true;
    else if (a->alarm && value < a->off_th)  a->alarm = false;
    any |= update_alarm(&flame, analogRead(flame.pin));
    digitalWrite(19, any);
```

Day 1 의 `button_t` 와 같은 설계입니다: **상태는 구조체에, 로직은 함수 하나에**. 센서가 늘어도 `alarm_t` 변수만 추가합니다. 불꽃 센서가 "가까울수록 작아지는" 제품이면 비교 방향을 바꿔야 합니다.
</details>

#### ✏️ 괄호 넣기 5-2

1. 켜는 임계값과 끄는 임계값을 다르게 두는 방식을 (　　　)라 한다.
2. MQ-2 AO 를 10kΩ 두 개로 분압하면 GPIO 전압은 AO 의 (　　　) 이 된다.
3. MQ-2 는 안의 (　　　)가 데워질 때까지 1~2분 예열이 필요하다.
4. 소리 센서는 한 번 읽은 값 대신 짧은 시간의 (　　　)값을 본다.

<details><summary>🔒 정답 보기 5-2</summary>

1. 히스테리시스 (hysteresis)
2. 절반 (1/2)
3. 히터
4. 최대 (최대 − 최소)
</details>

---

# 6교시 · 거리와 움직임 — 초음파 · PIR (15:00–15:50)

**학습 목표**
- HC-SR04 의 Trig · Echo 타이밍으로 거리를 계산하고 타임아웃을 처리한다
- 5V Echo 를 분압해 3.3V GPIO 를 보호한다
- PIR 로 움직임을 감지해 LED 와 연동한다

🎬 **D2-6 초음파 왕복 시간** — 물체 거리를 바꾸며 음파 왕복과 Echo 펄스 폭, 58µs = 1cm 관계 확인

<img src="img/d2_L6.svg" alt="6교시 배선">

> **Echo 분압** — HC-SR04 는 5V 부품이라 Echo 핀이 **5V 펄스**를 냅니다. **10kΩ 을 직렬로, 20kΩ(10kΩ 두 개 직렬)을 GND 로** 연결하면 GPIO 에는 5 × 20 ÷ 30 = **3.33V** 가 들어갑니다. Trig 는 ESP32 가 보내는 3.3V 로도 인식되므로 그대로 연결합니다.

### ▶ 예제 6-1 · 초음파 거리 — 소리의 왕복 시간

<img src="img/d2_ex6_1.svg" alt="예제 6-1 배선">

`ultrasonic.ino`
```c
const int PIN_TRIG = 17, PIN_ECHO = 16;

float read_cm(void) {
    digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);   // 10µs 펄스 = 측정 시작
    digitalWrite(PIN_TRIG, LOW);
    unsigned long us = pulseIn(PIN_ECHO, HIGH, 30000);     // 최대 30ms 기다림
    if (us == 0) return -1;                                // 타임아웃 = 측정 실패
    return us / 58.0f;                                     // 왕복 58µs ≈ 1cm
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
}

void loop() {
    float cm = read_cm();
    if (cm < 0) Serial.println("범위 밖");
    else        Serial.printf("cm:%.1f\n", cm);
    delay(100);
}
// → 손을 20cm 앞에 두면 cm:20.3 부근
```

소리는 1초에 약 343m, 즉 **1cm 를 29µs** 에 갑니다. Echo 펄스는 **갔다 오는 시간**이므로 1cm 당 약 **58µs** 입니다. `pulseIn` 의 세 번째 인자(타임아웃)를 빼면 물체가 없을 때 **1초 동안 멈춥니다**.

### ▶ 예제 6-2 · PIR — 사람의 움직임

<img src="img/d2_ex6_2.svg" alt="예제 6-2 배선">

`pir.ino`
```c
const int PIN_PIR = 13, PIN_LED = 25;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_LED, OUTPUT);
    Serial.println("PIR 예열 중 (약 30초 — 움직이지 마세요)");
    delay(30000);
    Serial.println("준비 완료");
}

void loop() {
    int m = digitalRead(PIN_PIR);
    digitalWrite(PIN_LED, m);
    Serial.printf("motion:%d\n", m);
    delay(100);
}
```

PIR(HC-SR501)은 **적외선(체온)의 변화**를 감지합니다. 가만히 서 있으면 감지하지 못하고, 움직여야 합니다. 전원을 넣은 뒤 **30초~1분 예열** 동안은 출력이 제멋대로입니다. 모듈의 두 가변저항은 **감도(거리)** 와 **HIGH 유지 시간**이고, 점퍼는 반복 감지 모드입니다.

### ▶ 예제 6-3 · 침입 감지 — 두 센서를 함께

<img src="img/d2_ex6_3.svg" alt="예제 6-3 배선">

`intruder.ino`
```c
// 예제 6-1 의 PIN_TRIG ~ read_cm 끝까지(setup 전까지)를 위에 붙여 넣는다
const int PIN_PIR = 13, PIN_LED = 25;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_LED, OUTPUT);
}

void loop() {
    bool moving = digitalRead(PIN_PIR);
    float cm = read_cm();
    bool near = (cm > 0 && cm < 30);
    digitalWrite(PIN_LED, moving && near);             // 움직임 + 30cm 이내
    Serial.printf("cm:%.0f,moving:%d,alert:%d\n", cm, moving * 50, (moving && near) * 100);
    delay(100);
}
```

PIR 은 넓게 "움직임이 있다"를, 초음파는 좁게 "얼마나 가까운가"를 봅니다. 두 조건을 **AND** 로 묶으면 오작동이 크게 줄어듭니다.

### ▶ 예제 6-4 · ⚠ 함정

| 한 일 | 결과 |
|---|---|
| Echo 를 **분압 없이** GPIO16 에 | 5V 펄스가 핀에 → **손상 위험**. 당장은 동작해서 더 위험 |
| HC-SR04 VCC 를 3V3 에 | 측정 거리가 짧아지거나 항상 타임아웃 |
| `pulseIn(PIN_ECHO, HIGH)` 타임아웃 없이 | 물체가 없으면 **loop 가 1초씩 멈춤** |
| 두 초음파 센서를 동시에 | 서로의 소리를 듣고 엉뚱한 값 — 번갈아 측정 |

#### ✏️ 빈칸 채우기 6-1

측정 실패(타임아웃)가 3번 연속이면 "센서 확인" 을 출력합니다.

```c
const int PIN_TRIG = 17, PIN_ECHO = 16;
int fail_count = 0;

float read_cm(void) {
    digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(____);
    digitalWrite(PIN_TRIG, LOW);
    unsigned long us = pulseIn(PIN_ECHO, HIGH, ____);
    if (us == 0) return -1;
    return us / ____;
}

void loop() {
    float cm = read_cm();
    if (cm < 0) {
        fail_count++;
        if (fail_count >= 3) Serial.println("센서 확인");
    } else {
        fail_count = ____;
        Serial.printf("cm:%.1f\n", cm);
    }
    delay(100);
}
```

<details><summary>🔒 정답 보기 6-1</summary>

```c
    digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
    unsigned long us = pulseIn(PIN_ECHO, HIGH, 30000);
    return us / 58.0f;
        fail_count = 0;
```

30000µs 타임아웃은 약 5m 왕복에 해당합니다(HC-SR04 측정 한계 약 4m). 성공하면 연속 실패 수를 0 으로 되돌립니다.
</details>

#### ✏️ 괄호 넣기 6-2

1. HC-SR04 측정을 시작하려면 Trig 에 (　　　)µs HIGH 펄스를 준다.
2. Echo 펄스 폭 약 (　　　)µs 가 거리 1cm 에 해당한다.
3. Echo 를 10kΩ : 20kΩ 로 분압하면 GPIO 전압은 약 (　　　)V 이다.
4. PIR 은 전원을 넣은 뒤 약 (　　　)초 예열이 필요하다.

<details><summary>🔒 정답 보기 6-2</summary>

1. 10
2. 58
3. 3.3
4. 30 (30~60)
</details>

---

# 7교시 · 수분 센서와 코드 모듈화 (16:00–16:50)

**학습 목표**
- 토양 · 수위 센서의 전원을 GPIO 로 켰다 꺼서 부식을 줄인다
- 코드를 `.h`(선언) 와 `.cpp`(정의) 로 나누고 `extern`, include guard 를 쓴다
- 컴파일과 링크 단계에서 분리된 파일이 어떻게 합쳐지는지 설명한다

🎬 **D2-7 헤더 분리 · 링크** — 세 파일이 각각 컴파일되어 .o 가 되고 링커가 합치는 과정, 잴 때만 켜는 전원 타이밍

<img src="img/d2_L7.svg" alt="7교시 배선">

> **센서 전원을 GPIO 로** — 토양 · 수위 센서는 전극에 계속 전기가 흐르면 **전기 분해로 부식**됩니다. V 핀을 3V3 레일 대신 **GPIO(14, 23)** 에 연결해 **잴 때만 잠깐 켭니다**. 센서 전류가 수 mA 라 GPIO 로 충분합니다. 센서는 **물에 닿는 부분만** 담그고 기판 윗부분(부품이 있는 쪽)은 젖지 않게 합니다.

### ▶ 예제 7-1 · 토양 습도 — 잴 때만 켠다

<img src="img/d2_ex7_1.svg" alt="예제 7-1 배선">

`soil.ino`
```c
const int PIN_SOIL = 34, PIN_SOIL_PWR = 14;

int read_soil(void) {
    digitalWrite(PIN_SOIL_PWR, HIGH);      // 전원 ON
    delay(10);                             // 안정될 때까지 잠깐
    int v = analogRead(PIN_SOIL);
    digitalWrite(PIN_SOIL_PWR, LOW);       // 바로 OFF
    return v;
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_SOIL_PWR, OUTPUT);
    digitalWrite(PIN_SOIL_PWR, LOW);
}

void loop() {
    Serial.printf("soil:%d\n", read_soil());
    delay(1000);                           // 1초에 10ms 만 켜짐 → 1%
}
// → 공기 중: 0 근처 / 젖은 휴지나 물컵: 값이 크게 오른다
```

### ▶ 예제 7-2 · 수위 — 같은 모양의 함수

<img src="img/d2_ex7_2.svg" alt="예제 7-2 배선">

`water.ino`
```c
const int PIN_WATER = 35, PIN_WATER_PWR = 23;

int read_water(void) {
    digitalWrite(PIN_WATER_PWR, HIGH);
    delay(10);
    int v = analogRead(PIN_WATER);
    digitalWrite(PIN_WATER_PWR, LOW);
    return v;
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_WATER_PWR, OUTPUT);
}

void loop() {
    Serial.printf("water:%d\n", read_water());
    delay(1000);
}
// → 센서를 물에 조금씩 담글수록 값이 계단식으로 오른다
```

`read_soil` 과 `read_water` 는 **핀 번호만 다르고 똑같습니다**. 이런 코드가 늘어나면 한 파일이 길어집니다 — 다음 예제에서 파일로 나눕니다.

### ▶ 예제 7-3 · sensor.h / sensor.cpp 로 분리

<img src="img/d2_ex7_3.svg" alt="예제 7-3 배선">

**IDE 에서 파일 추가하기**: 스케치 이름 탭 오른쪽의 **⋯ (점 세 개) → 새 탭** 을 누르고 이름을 `sensor.h` 로 입력합니다. 같은 방법으로 `sensor.cpp` 를 만듭니다. 세 파일이 **같은 스케치 폴더**에 생깁니다.

```text
water_soil\
 ├─ water_soil.ino
 ├─ sensor.h
 └─ sensor.cpp
```

`sensor.h` — **선언**: "이런 것이 있다"
```c
#ifndef SENSOR_H          // include guard: 두 번 포함돼도 한 번만
#define SENSOR_H

typedef struct {
    int pin;              // 아날로그 입력
    int pwr;              // 전원 GPIO
    const char *name;
} powered_sensor_t;

void sensor_init(const powered_sensor_t *s);
int  sensor_read(const powered_sensor_t *s);

extern int sensor_read_count;   // 다른 파일에 정의된 전역 변수

#endif
```

`sensor.cpp` — **정의**: "실제로 이렇게 한다"
```c
#include <Arduino.h>
#include "sensor.h"

int sensor_read_count = 0;       // 정의는 딱 한 곳에만

void sensor_init(const powered_sensor_t *s) {
    pinMode(s->pwr, OUTPUT);
    digitalWrite(s->pwr, LOW);
}

int sensor_read(const powered_sensor_t *s) {
    digitalWrite(s->pwr, HIGH);
    delay(10);
    int v = analogRead(s->pin);
    digitalWrite(s->pwr, LOW);
    sensor_read_count++;
    return v;
}
```

`water_soil.ino` — **사용**
```c
#include "sensor.h"

const powered_sensor_t soil  = {34, 14, "soil"};
const powered_sensor_t water = {35, 23, "water"};

void setup() {
    Serial.begin(115200);
    sensor_init(&soil);
    sensor_init(&water);
}

void loop() {
    Serial.printf("soil:%d,water:%d\n", sensor_read(&soil), sensor_read(&water));
    if (sensor_read_count % 20 == 0) Serial.printf("# reads=%d\n", sensor_read_count);
    delay(1000);
}
```

| 파일 | 담는 것 | 컴파일 결과 |
|---|---|---|
| `sensor.h` | 타입, 함수 원형, `extern` 선언 | 따로 컴파일되지 않음 — `#include` 된 곳에 복사됨 |
| `sensor.cpp` | 함수 몸체, 전역 변수 정의 | `sensor.cpp.o` |
| `water_soil.ino` | `setup` · `loop` | `water_soil.ino.cpp.o` |
| 링크 | 두 `.o` 의 이름을 서로 연결 | `.elf` → `.bin` (Day 1 2교시) |

C 과정 Day 1 의 **전처리 → 컴파일 → 링크**가 그대로입니다. 자세한 출력(verbose)을 켜 두었다면 빌드 로그에서 `sensor.cpp.o` 가 따로 만들어지는 것을 확인할 수 있습니다.

### ▶ 예제 7-4 · ⚠ 함정 — 링크 오류 읽기

| 한 일 | 오류 | 원인 |
|---|---|---|
| `sensor.h` 에 `int sensor_read_count = 0;` (정의) | `multiple definition of 'sensor_read_count'` | 헤더를 포함한 **파일마다** 정의가 생김 → 헤더에는 `extern` 선언만 |
| `sensor.cpp` 에서 `int sensor_read_count = 0;` 를 지움 | `undefined reference to 'sensor_read_count'` | 선언만 있고 **정의가 없음** |
| include guard 없이 헤더를 두 번 포함 | `redefinition of 'struct …'` / `conflicting declaration` | 같은 `typedef` 가 두 번 |
| 탭 이름을 `sensor.c` 로 만듦 | `.c` 는 C 로, `.ino` 는 C++ 로 컴파일되어 이름 규칙이 달라 **undefined reference** | 오늘은 `.cpp` 로 통일 |

`multiple definition`, `undefined reference` 는 **컴파일러가 아니라 링커**가 내는 오류입니다. 오류 메시지에 `ld` 또는 `collect2` 가 보이면 링크 단계입니다.

#### ✏️ 빈칸 채우기 7-1

헤더 파일을 완성합니다.

```c
#____ SENSOR_H
#define SENSOR_H

typedef struct { int pin; int pwr; const char *name; } powered_sensor_t;

void sensor_init(const powered_sensor_t *s);
int  sensor_read(const powered_sensor_t *s);

____ int sensor_read_count;

#____
```

<details><summary>🔒 정답 보기 7-1</summary>

```c
#ifndef SENSOR_H
extern int sensor_read_count;
#endif
```

`#ifndef` 로 시작해 `#endif` 로 닫는 것이 include guard 입니다. `extern` 은 "메모리는 다른 곳에 있다"는 선언이라 여러 파일에 포함되어도 충돌하지 않습니다.
</details>

#### ✏️ 괄호 넣기 7-2

1. 토양 센서의 부식을 줄이려고 V 핀을 (　　　)에 연결해 잴 때만 켠다.
2. 함수 원형과 `extern` 선언은 (`.h` / `.cpp`) 파일에, 함수 몸체는 (`.h` / `.cpp`) 파일에 둔다.
3. `undefined reference` 오류는 (`컴파일러` / `링커`) 가 낸다.
4. 헤더가 두 번 포함돼도 한 번만 처리되게 하는 장치를 (　　　)라 한다.

<details><summary>🔒 정답 보기 7-2</summary>

1. GPIO
2. .h / .cpp
3. 링커
4. include guard
</details>

---

# 8교시 · 미니 프로젝트 — 다중 센서 모니터 · 정리 (17:00–17:50)

**학습 목표**
- 센서 구조체 배열과 함수 포인터로 여러 센서를 같은 코드로 순회한다
- 시리얼 플로터와 CSV 로그 두 가지 형식으로 출력한다

🎬 **D2-8 CSV → 시리얼 플로터** — 센서 배열을 돌며 한 줄이 만들어지고 그래프에 찍히는 과정

<img src="img/d2_L8.svg" alt="8교시 배선">

### ▶ 예제 8-1 · 센서를 표로 — 함수 포인터 배열

Day 1 8교시의 `table[current]()` 처럼, 센서마다 **읽는 함수의 주소**를 구조체에 넣어 두면 `loop` 는 표를 돌기만 하면 됩니다.

```c
typedef struct {
    const char *name;
    float (*read)(void);      // 읽는 함수 — 센서마다 다름
    float value;              // 마지막 값
} sensor_t;

float read_pot(void)   { return analogRead(34); }
float read_light(void) { return analogRead(35); }

sensor_t sensors[] = {
    {"pot",   read_pot,   0},
    {"light", read_light, 0},
};
const int NS = sizeof(sensors) / sizeof(sensors[0]);

void loop() {
    for (int i = 0; i < NS; i++) {
        sensors[i].value = sensors[i].read();     // 함수 포인터 호출
        Serial.printf("%s%s:%.1f", i ? "," : "", sensors[i].name, sensors[i].value);
    }
    Serial.println();
    delay(200);
}
// → pot:2041.0,light:1733.0
```

### ✏️ 종합 과제 8-A · 다중 센서 모니터

<img src="img/d2_ex8_A.svg" alt="종합 과제 8-A 배선">

가변저항(34) · 조도(35) · LM35(33) · DHT11(27) 네 센서를 표로 관리합니다. 시리얼로 `m` 을 보내면 **플로터 형식 ↔ CSV 형식**이 바뀝니다.

`multi_monitor.ino`
```c
#include <DHT.h>

DHT dht(27, DHT11);

typedef struct {
    const char *name;
    float (*read)(void);
    float value;
    uint32_t period_ms;       // 이 센서를 읽는 주기
    uint32_t last_ms;         // 마지막으로 읽은 시각
} sensor_t;

/* ---------- 읽기 함수 ---------- */
float read_pot(void)   { return analogRead(34); }
float read_light(void) { return analogRead(35); }
float read_lm35(void) {
    long s = 0;
    for (int i = 0; i < 16; i++) s += analogReadMilliVolts(33);
    return s / 16.0f / 10.0f;
}
float read_dht_t(void) {
    // TODO 1: dht.readTemperature() 를 읽어 NaN 이면 -99 를 돌려준다
}

/* ---------- 센서 표 ---------- */
sensor_t sensors[] = {
    {"pot",   read_pot,   0, 100,  0},
    {"light", read_light, 0, 100,  0},
    {"lm35",  read_lm35,  0, 500,  0},
    // TODO 2: DHT 온도를 2000ms 주기로 추가
};
const int NS = sizeof(sensors) / sizeof(sensors[0]);
bool csv_mode = false;

void setup() {
    Serial.begin(115200);
    dht.begin();
}

void loop() {
    uint32_t now = millis();
    // TODO 3: 각 센서의 주기가 지났으면 read() 로 값을 갱신하고 last_ms 를 now 로

    // 모드 전환: 시리얼 모니터에서 m 을 보내면
    if (Serial.available() && Serial.read() == 'm') {
        csv_mode = !csv_mode;
        if (csv_mode) {
            // TODO 4: CSV 머리줄 출력 — "ms,pot,light,lm35,dht_t"
        }
    }

    // TODO 5: csv_mode 이면 "ms,값,값,…", 아니면 "이름:값,이름:값,…" 한 줄 출력

    delay(100);
}
```

기대 출력:
```text
pot:2041.0,light:1733.0,lm35:24.6,dht_t:25.0          ← 플로터 모드
ms,pot,light,lm35,dht_t                               ← m 을 보낸 뒤
15230,2040.0,1731.0,24.6,25.0
15330,2043.0,1730.0,24.6,25.0
```

<details><summary>🔒 정답 보기 8-A (리뷰 시간에 함께 엽니다)</summary>

```c
float read_dht_t(void) {
    float t = dht.readTemperature();
    return isnan(t) ? -99 : t;
}

    {"dht_t", read_dht_t, 0, 2000, 0},

    for (int i = 0; i < NS; i++) {
        if (now - sensors[i].last_ms >= sensors[i].period_ms) {
            sensors[i].value = sensors[i].read();
            sensors[i].last_ms = now;
        }
    }

        if (csv_mode) {
            Serial.print("ms");
            for (int i = 0; i < NS; i++) Serial.printf(",%s", sensors[i].name);
            Serial.println();
        }

    if (csv_mode) {
        Serial.printf("%lu", now);
        for (int i = 0; i < NS; i++) Serial.printf(",%.1f", sensors[i].value);
    } else {
        for (int i = 0; i < NS; i++)
            Serial.printf("%s%s:%.1f", i ? "," : "", sensors[i].name, sensors[i].value);
    }
    Serial.println();
```
</details>

**확장 과제 (선택)**
- 가변저항 · 조도 값에 2교시 **링 버퍼 필터** 적용 (`sensor_t` 에 `ring_t` 멤버 추가)
- 5교시 `alarm_t` 를 붙여 LM35 가 30°C 를 넘으면 경보
- CSV 출력을 복사해 엑셀에서 그래프 그리기 (Day 4 에서는 파일로 저장합니다)

**제출 체크리스트**
- [ ] 네 센서 값이 한 줄에 나오고 시리얼 플로터에 네 개의 선이 그려진다
- [ ] DHT 는 2초마다, 나머지는 더 자주 갱신된다 (`delay` 로 전체를 늦추지 않음)
- [ ] `m` 으로 CSV 모드가 바뀌고 머리줄이 한 번 나온다
- [ ] 센서 추가가 **표에 한 줄 추가**로 끝나는 구조다

### 오늘의 여섯 문장

1. ADC 는 12비트(0~4095). 전압이 필요하면 `analogReadMilliVolts`, 핀은 ADC1(32~39) (D2-1)
2. 저항 센서는 10kΩ 과 분압해서 읽고, 흔들림은 링 버퍼 이동평균으로 줄인다 (D2-2)
3. 서보는 50Hz 펄스 폭(0.5~2.5ms). 조이스틱은 3V3, 서보는 5V, GND 는 공통 (D2-3)
4. 5V 출력(MQ-2 AO, HC-SR04 Echo)은 반드시 분압. LM35 전원은 5V, 출력은 그대로 (D2-4 · D2-6)
5. 경보는 히스테리시스로 떨리지 않게, 실패(NaN · 타임아웃)는 반드시 검사 (D2-5 · D2-6)
6. 선언은 `.h`, 정의는 `.cpp`, 공유 변수는 `extern`. 센서는 구조체 + 함수 포인터 표로 (D2-7 · D2-8)

- 개념 워크시트: `anim/worksheet.html` 8교시까지 채점 후 결과 코드 제출
- 제출: `multi_monitor.ino` + 시리얼 플로터 캡처 + 워크시트 캡처
- **Day 3 예고**: 7-Segment, 74HC595 시프트 레지스터, 4자리 멀티플렉싱, 8×8 도트 매트릭스, I2C(1602 LCD · PCF8591), DC 모터 · 스테퍼 · 릴레이. 오늘의 센서 값을 **LCD 와 7-Segment 에 표시**하고 **모터를 움직입니다**
