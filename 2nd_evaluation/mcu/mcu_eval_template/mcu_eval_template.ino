/* mcu_eval_template.ino — 2차 평가 · MCU 동작 원리 실습 (정규 평가)
 *
 * 환경 : Arduino IDE 2.x · esp32 by Espressif 3.x (ESP32 Dev Module) · 시리얼 모니터 115200 baud
 * 배선 : (Day 1 그림 1 과 같은 자리 — 부품 4개 사용)
 *        GPIO25 → 220Ω → LED 빨강(+, 긴 다리) → GND
 *        GPIO27 → 220Ω → LED 초록(+, 긴 다리) → GND
 *        GPIO4  → 버튼 → GND                (대각선 두 다리, 내부 풀업 사용)
 *        GPIO23 → Passive Buzzer(+) → GND   (뒷면에 기판이 보이는 부저)
 * 사용법: 맨 위 STEP 값을 1 → 6 으로 바꿔 가며 업로드(Ctrl+U)하고 시리얼 모니터(Ctrl+Shift+M)를 본다.
 *        각 단계 끝의 [N단계] 결과 와 [N단계] 기대값 이 같고 부품이 설명대로 동작하면 완료.
 *        결과가 안 보이면 보드의 EN 버튼을 한 번 누른다.
 *
 * 규칙 : 빈칸 ______ 8곳과 TODO 3곳만 채운다. [N단계] 결과 / 기대값 출력 코드는 고치지 않는다.
 *        STEP 으로 고른 단계만 컴파일되므로, 못 채운 단계가 있어도 다른 단계는 진행할 수 있다.
 *        (단, setup() 의 빈칸 ③ 은 모든 단계에 필요하므로 1단계에서 먼저 채운다)
 *
 *  단계  주제                                    Day 1   형태
 *   1    출력 — LED 2개 번갈아 깜빡이기             3교시   빈칸 3
 *   2    입력 — 풀업 버튼 읽기                     5교시   빈칸 2
 *   3    버튼으로 LED 토글                         5교시   TODO 1
 *   4    PWM — 밝기 서서히 바꾸기                  7교시   빈칸 2
 *   5    버튼으로 밝기 단계 바꾸기                 7교시   TODO 2
 *   6    Passive Buzzer 멜로디 + 초록 LED           7교시   빈칸 1 + TODO 3
 */
#define STEP 1                      // ← 1 ~ 6 으로 바꿔 가며 업로드

const int PIN_LED  = 25;            // LED 빨강
const int PIN_LED2 = 27;            // LED 초록
const int PIN_BTN  = 4;             // 버튼
const int PIN_BUZ  = 23;            // Passive Buzzer

#if STEP == 1
/* ───────────── 1단계: LED 2개 번갈아 깜빡이기 ───────────── */
void run_step() {
  pinMode(PIN_LED, ______);                    // ① 빨강 LED 핀을 출력으로
  pinMode(PIN_LED2, OUTPUT);
  for (int i = 1; i <= 5; i++) {
    digitalWrite(PIN_LED, ______);             // ② 빨강 켜기 (3.3V 출력)
    digitalWrite(PIN_LED2, LOW);
    delay(400);
    digitalWrite(PIN_LED, LOW);
    digitalWrite(PIN_LED2, HIGH);              // 초록 켜기
    delay(400);
    Serial.printf("교대 %d\n", i);
  }
  digitalWrite(PIN_LED2, LOW);
  Serial.println("[1단계] 결과 : 빨강·초록 교대 5회 완료");
  Serial.println("[1단계] 기대값: 빨강·초록 교대 5회 완료 (두 LED 가 0.4초씩 번갈아 5번 켜짐)");
}

#elif STEP == 2
/* ───────────── 2단계: 풀업 버튼 읽기 ─────────────
 * 10초 동안 버튼을 읽어, 누르는 동안만 빨강 LED 를 켜고 눌린 횟수를 센다. */
void run_step() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, ______);                    // ① 내부 풀업 입력 모드
  int idle = digitalRead(PIN_BTN);             // 시작할 때(안 누름) 값
  Serial.println("10초 동안 버튼을 3번 누르세요 (누르는 동안만 LED ON)");

  int last = HIGH, presses = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 10000) {
    int now = digitalRead(PIN_BTN);
    digitalWrite(PIN_LED, now == ______ ? HIGH : LOW);   // ② 버튼이 "눌린" 값일 때만 LED ON
    if (last == HIGH && now == LOW) {
      presses++;
      Serial.printf("눌림 %d\n", presses);
    }
    last = now;
    delay(30);                                 // 간단한 디바운스
  }
  digitalWrite(PIN_LED, LOW);
  Serial.printf("[2단계] 결과 : 안 누름 값=%d, 눌림 %d회\n", idle, presses);
  Serial.println("[2단계] 기대값: 안 누름 값=1, 눌림 3회 (3번 눌렀을 때)");
}

#elif STEP == 3
/* ───────────── 3단계: 버튼으로 LED 토글 ─────────────
 * 버튼을 누를 때마다 빨강 LED 를 켜고/끄기를 바꾼다. 5번 누르면 끝낸다. */
void run_step() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  Serial.println("버튼을 5번 누르세요 (누를 때마다 LED 토글)");

  int last = HIGH, led = 0, count = 0;
  while (count < 5) {
    int now = digitalRead(PIN_BTN);
    // TODO 1: last 가 HIGH 이고 now 가 LOW 인 순간(눌리는 순간)에만
    //   ① count 를 1 늘린다
    //   ② led 를 반전(!)하고 digitalWrite 로 빨강 LED 에 출력한다
    //   ③ Serial.printf 로 "토글 #count LED ON" 또는 "토글 #count LED OFF" 를 출력한다

    last = now;
    delay(30);                                 // 간단한 디바운스
  }
  Serial.printf("[3단계] 결과 : count=%d, led=%d\n", count, led);
  Serial.println("[3단계] 기대값: count=5, led=1 (5번 토글 → 켜진 상태로 끝남)");
}

#elif STEP == 4
/* ───────────── 4단계: PWM 으로 밝기 바꾸기 ─────────────
 * 듀티를 0 → 255 → 0 으로 바꾸며 빨강 LED 를 서서히 밝게/어둡게 하기를 2번 반복한다. */
void run_step() {
  ledcAttach(PIN_LED, 5000, ______);           // ① 분해능(비트 수) — 듀티 범위가 0~255 가 되도록
  int maxDuty = 0;
  for (int rep = 0; rep < 2; rep++) {
    for (int d = 0; d <= 255; d += 5) {
      ledcWrite(PIN_LED, ______);              // ② 지금 듀티 값 출력
      if (d > maxDuty) maxDuty = d;
      delay(20);
    }
    for (int d = 255; d >= 0; d -= 5) {
      ledcWrite(PIN_LED, d);
      delay(20);
    }
  }
  Serial.printf("[4단계] 결과 : 최대 듀티=%d (%.0f%%)\n", maxDuty, maxDuty * 100.0 / 255);
  Serial.println("[4단계] 기대값: 최대 듀티=255 (100%) — LED 가 서서히 밝아졌다 어두워지기를 2번 반복");
}

#elif STEP == 5
/* ───────────── 5단계: 버튼으로 밝기 단계 바꾸기 ─────────────
 * 밝기 표 LEVELS 를 버튼을 누를 때마다 다음 단계로 바꾼다. 마지막 단계 다음은 처음(0)으로. 4번 누르면 끝낸다. */
const int LEVELS[] = {0, 64, 128, 255};
const int N_LEVEL = sizeof(LEVELS) / sizeof(LEVELS[0]);

void run_step() {
  pinMode(PIN_BTN, INPUT_PULLUP);
  ledcAttach(PIN_LED, 5000, 8);
  int idx = 0, last = HIGH, count = 0;
  ledcWrite(PIN_LED, LEVELS[idx]);
  Serial.println("버튼을 4번 누르세요 (누를 때마다 밝기 단계 변경)");

  while (count < 4) {
    int now = digitalRead(PIN_BTN);
    if (last == HIGH && now == LOW) {
      count++;
      // TODO 2:
      //   ① idx 를 다음 단계로 바꾼다 — 마지막(N_LEVEL-1) 다음은 0 (% 사용)
      //   ② LEVELS[idx] 를 ledcWrite 로 LED 에 출력한다
      //   ③ Serial.printf 로 "단계 idx → 듀티 값" 을 출력한다

    }
    last = now;
    delay(30);                                 // 간단한 디바운스
  }
  Serial.printf("[5단계] 결과 : idx=%d, duty=%d\n", idx, LEVELS[idx]);
  Serial.println("[5단계] 기대값: idx=0, duty=0 (출력 순서 64 → 128 → 255 → 0)");
}

#elif STEP == 6
/* ───────────── 6단계: Passive Buzzer 멜로디 + 초록 LED ─────────────
 * 버튼을 누르면 초록 LED 를 켜고 '도 레 미 파 솔' 을 연주한 뒤 LED 를 끈다. 2번 반복하면 끝낸다.
 * 악보는 {주파수, 길이} 구조체 배열(데이터), 연주는 반복문(코드)이다. */
typedef struct { uint16_t freq; uint16_t ms; } melody_t;
const melody_t MELODY[] = { {262, 200}, {294, 200}, {330, 200}, {349, 200}, {392, 400} };
const int N_NOTE = sizeof(MELODY) / sizeof(MELODY[0]);

void run_step() {
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED2, OUTPUT);
  ledcAttach(PIN_BUZ, 1000, 10);
  Serial.println("버튼을 2번 누르세요 (누를 때마다 초록 LED + 멜로디)");

  int last = HIGH, plays = 0, notes = 0;
  while (plays < 2) {
    int now = digitalRead(PIN_BTN);
    if (last == HIGH && now == LOW) {
      plays++;
      digitalWrite(PIN_LED2, HIGH);
      // TODO 3: MELODY 의 N_NOTE 개 음을 차례로 연주한다
      //   ① for 문으로 i = 0 ~ N_NOTE-1 을 돈다
      //   ② ledcWriteTone(PIN_BUZ, MELODY[i].freq) 로 소리를 내고 delay(MELODY[i].ms) 만큼 기다린다
      //   ③ 한 음을 낼 때마다 notes 를 1 늘린다

      ______(PIN_BUZ, 0);                      // ① 0 Hz = 소리 끄기 (주파수로 소리를 내는 함수)
      digitalWrite(PIN_LED2, LOW);
      Serial.printf("연주 #%d 끝\n", plays);
    }
    last = now;
    delay(30);
  }
  Serial.printf("[6단계] 결과 : 연주 %d회, 낸 음 %d개\n", plays, notes);
  Serial.println("[6단계] 기대값: 연주 2회, 낸 음 10개 (도레미파솔 × 2, 연주하는 동안만 초록 LED ON)");
}
#endif

void setup() {
  Serial.begin(______);                        // ③ 시리얼 모니터와 같은 통신 속도 (1단계에서 채운다)
  delay(500);
  Serial.printf("\n=== 2차 평가 MCU · STEP %d ===\n", STEP);
  run_step();
}

void loop() {
}
