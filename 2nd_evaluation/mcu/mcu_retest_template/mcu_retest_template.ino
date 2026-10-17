/* mcu_retest_template.ino — 2차 평가 · MCU 동작 원리 실습 (재평가용 대체 과제)
 *
 * 환경 : Arduino IDE 2.x · esp32 by Espressif 3.x (ESP32 Dev Module) · 시리얼 모니터 115200 baud
 * 배선 : (Day 2 1교시 가변저항 + Day 1 LED·부저 — 부품 4개 사용, 5V 레일은 쓰지 않는다)
 *        가변저항: 양 끝 다리 → 3V3 · GND,  가운데 다리(와이퍼) → GPIO34 (ADC1)
 *        GPIO25 → 220Ω → LED 빨강(+) → GND
 *        GPIO27 → 220Ω → LED 초록(+) → GND
 *        GPIO23 → Passive Buzzer(+) → GND
 * 사용법: STEP 값을 1 → 7 로 바꿔 가며 업로드하고 시리얼 모니터를 본다.
 *        각 단계는 10초 동안 0.5초마다 값을 출력한다. 손잡이를 끝에서 끝까지 천천히 돌려 본다.
 *
 * 7단계 · 빈칸 ______ 8곳 + TODO 3곳 · 난이도 3구간
 *   [쉬움   1~3, 7] 빈칸 채우기 (1·2단계 각 1곳, 3단계 2곳, 7단계 3곳)
 *   [보통   4     ] 빈칸 1곳 + TODO 1 (한 줄)
 *   [어려움 5~6   ] TODO 2 (함수) · TODO 3 (조건문)
 * [N단계] 결과 / 기대값 출력 코드는 고치지 않는다.
 */
#define STEP 1                      // ← 1 ~ 7 로 바꿔 가며 업로드

const int PIN_POT = 34;             // ADC1 — Wi-Fi 를 켜도 동작, 입력 전용 핀
const int PIN_LED = 25;             // LED 빨강
const int PIN_LED2 = 27;            // LED 초록
const int PIN_BUZ = 23;             // Passive Buzzer

int g_min = 4095, g_max = 0;        // 10초 동안 본 raw 의 최소 · 최대 (확인용)

// 제공 함수 — 고치지 않는다
void track(int raw) {
  if (raw < g_min) g_min = raw;
  if (raw > g_max) g_max = raw;
}

#if STEP == 1
/* ───────────── [쉬움] 1단계: analogRead ───────────── */
void run_step() {
  uint32_t t0 = millis();
  while (millis() - t0 < 10000) {
    int raw = ______(PIN_POT);                 // ① ADC 원시 값(0~4095) 읽기
    track(raw);
    Serial.printf("raw=%4d\n", raw);
    delay(500);
  }
  Serial.printf("[1단계] 결과 : 최소=%d, 최대=%d\n", g_min, g_max);
  Serial.println("[1단계] 기대값: 최소=0 근처(수십 이하), 최대=4095 (끝에서 끝까지 돌렸을 때)");
}

#elif STEP == 2
/* ───────────── [쉬움] 2단계: 보정된 전압(mV) ───────────── */
void run_step() {
  int mv_max = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 10000) {
    int raw = analogRead(PIN_POT);
    int mv  = ______(PIN_POT);                 // ① 칩 보정값으로 계산한 mV 읽기
    track(raw);
    if (mv > mv_max) mv_max = mv;
    Serial.printf("raw=%4d  mv=%4d\n", raw, mv);
    delay(500);
  }
  Serial.printf("[2단계] 결과 : 최대 mV=%d\n", mv_max);
  Serial.println("[2단계] 기대값: 최대 mV=약 3100~3300 (3.3V 쪽 끝까지 돌렸을 때)");
}

#elif STEP == 3
/* ───────────── [쉬움] 3단계: 기준값으로 LED 켜기 ─────────────
 * raw 가 ADC 범위의 절반(2048)보다 크면 LED ON, 아니면 OFF */
void run_step() {
  pinMode(PIN_LED, ______);                    // ① LED 핀을 출력으로
  int on_count = 0, off_count = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 10000) {
    int raw = analogRead(PIN_POT);
    int on = raw > ______;                     // ② 기준값 (12비트 범위의 절반)
    digitalWrite(PIN_LED, on ? HIGH : LOW);
    if (on) on_count++; else off_count++;
    Serial.printf("raw=%4d  LED %s\n", raw, on ? "ON" : "OFF");
    delay(500);
  }
  Serial.printf("[3단계] 결과 : ON 표본=%d, OFF 표본=%d\n", on_count, off_count);
  Serial.println("[3단계] 기대값: ON 표본>0, OFF 표본>0 (손잡이 위치에 따라 LED 가 켜지고 꺼짐)");
}

#elif STEP == 4
/* ───────────── [보통] 4단계: raw → PWM 듀티 ─────────────
 * 12비트 raw(0~4095)를 8비트 듀티(0~255)로 바꿔 LED 밝기로 출력한다. */
void run_step() {
  ledcAttach(PIN_LED, 5000, ______);           // ① 분해능 — 듀티 범위가 0~255 가 되는 비트 수
  int duty_max = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 10000) {
    int raw = analogRead(PIN_POT);
    int duty = 0;
    // TODO 1: raw(0~4095) 를 duty(0~255) 로 바꾼다 — 16 으로 나누거나 오른쪽으로 4비트 시프트 (한 줄)

    ledcWrite(PIN_LED, duty);
    if (duty > duty_max) duty_max = duty;
    Serial.printf("raw=%4d  duty=%3d\n", raw, duty);
    delay(500);
  }
  Serial.printf("[4단계] 결과 : 최대 듀티=%d\n", duty_max);
  Serial.println("[4단계] 기대값: 최대 듀티=255 (손잡이를 돌리는 만큼 LED 밝기가 바뀜)");
}

#elif STEP == 5
/* ───────────── [어려움] 5단계: n 번 읽어 평균 ─────────────
 * 손잡이를 가만히 두어도 raw 는 수십씩 흔들린다. 여러 번 읽어 평균하면 흔들림이 줄어든다. */
int read_avg(int pin, int n) {
  // TODO 2: analogRead(pin) 을 n 번 읽어 합(long)을 구한 뒤 평균(합 / n)을 반환한다
  return 0;
}

void run_step() {
  Serial.println("손잡이를 가운데쯤에 두고 손을 떼세요");
  delay(2000);
  int r_min = 4095, r_max = 0, a_min = 4095, a_max = 0;
  for (int i = 0; i < 20; i++) {
    int raw = analogRead(PIN_POT);
    int avg = read_avg(PIN_POT, 32);
    if (raw < r_min) r_min = raw;
    if (raw > r_max) r_max = raw;
    if (avg < a_min) a_min = avg;
    if (avg > a_max) a_max = avg;
    Serial.printf("raw=%4d  avg32=%4d\n", raw, avg);
    delay(500);
  }
  Serial.printf("[5단계] 결과 : 평균값>0=%d, 원시 흔들림=%d, 평균 흔들림=%d, 평균이 더 안정=%d\n",
                a_min > 0, r_max - r_min, a_max - a_min, (a_max - a_min) <= (r_max - r_min));
  Serial.println("[5단계] 기대값: 평균값>0=1, 평균이 더 안정=1 (평균 흔들림 ≤ 원시 흔들림)");
}

#elif STEP == 6
/* ───────────── [어려움] 6단계: 히스테리시스 ─────────────
 * 기준값 하나(2048)만 쓰면 경계에서 LED 가 떨린다. 켜는 기준과 끄는 기준을 다르게 둔다.
 *   꺼져 있을 때: raw > 3000 이면 켠다
 *   켜져 있을 때: raw < 1000 이면 끈다
 *   그 사이(1000~3000)에서는 지금 상태를 그대로 유지한다 */
const int TH_ON = 3000, TH_OFF = 1000;

void run_step() {
  pinMode(PIN_LED, OUTPUT);
  Serial.println("10초 동안 손잡이를  0쪽 끝 → 반대쪽 끝 → 가운데(잠시 멈춤) → 0쪽 끝  순서로 돌리세요");
  int led = 0, changes = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 10000) {
    int raw = analogRead(PIN_POT);
    int prev = led;
    // TODO 3: 위 규칙대로 led 값(0/1)을 정한다 (if / else if 두 줄 정도)

    if (led != prev) changes++;
    digitalWrite(PIN_LED, led);
    Serial.printf("raw=%4d  LED %s\n", raw, led ? "ON" : "OFF");
    delay(500);
  }
  Serial.printf("[6단계] 결과 : 상태 변경 %d회, 마지막 LED=%s\n", changes, led ? "ON" : "OFF");
  Serial.println("[6단계] 기대값: 상태 변경 2회, 마지막 LED=OFF (가운데에서 멈춘 동안에는 ON 유지)");
}

#elif STEP == 7
/* ───────────── [쉬움] 7단계: 가변저항 → 부저 음 높이 + 초록 경고 LED ─────────────
 * 손잡이를 돌리면 부저 음이 200 Hz ~ 1223 Hz 로 바뀐다. raw 가 3000 보다 크면 초록 LED 를 켠다. */
void run_step() {
  pinMode(PIN_LED2, ______);                   // ① 초록 LED 핀을 출력으로
  ledcAttach(PIN_BUZ, 1000, 10);
  int f_max = 0, warn = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 10000) {
    int raw = analogRead(PIN_POT);
    int freq = 200 + raw / 4;                  // 0~4095 → 200~1223 Hz
    ______(PIN_BUZ, freq);                     // ② 주파수로 부저 소리를 내는 함수
    digitalWrite(PIN_LED2, raw > ______ ? HIGH : LOW);   // ③ 경고 기준값
    if (freq > f_max) f_max = freq;
    if (raw > 3000) warn++;
    Serial.printf("raw=%4d  freq=%4d Hz  %s\n", raw, freq, raw > 3000 ? "경고" : "");
    delay(500);
  }
  ledcWriteTone(PIN_BUZ, 0);                   // 소리 끄기
  digitalWrite(PIN_LED2, LOW);
  Serial.printf("[7단계] 결과 : 최고 주파수=%d Hz, 경고 표본>0=%d\n", f_max, warn > 0);
  Serial.println("[7단계] 기대값: 최고 주파수=1223 Hz, 경고 표본>0=1 (끝까지 돌렸을 때 초록 LED ON)");
}
#endif

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.printf("\n=== 2차 재평가 MCU · STEP %d ===\n", STEP);
  run_step();
}

void loop() {
}
