/* esp32_node.ino — Day 4 게이트웨이의 ESP32 쪽 (MCU 과정 보드 · Arduino core 3.x)
 *
 *   DHT11 DATA → GPIO27 (MCU Day 2 와 같은 배선) · LED → 220Ω → GPIO25
 *   Serial2: RX2 = GPIO16 ← Pi TXD(GPIO14) · TX2 = GPIO17 → Pi RXD(GPIO15) · GND 공통
 *   1초마다 "T=23.0,H=45*CS" 를 보내고, "LED=1*CS" 를 받으면 LED 를 켠다
 */
#include <DHT.h>

const int PIN_DHT = 27, PIN_LED = 25;
DHT dht(PIN_DHT, DHT11);

uint8_t cs_xor(const char *s) {
  uint8_t x = 0;
  while (*s && *s != '*') x ^= (uint8_t)*s++;
  return x;
}

void send_line(const char *body) {
  char buf[64];
  snprintf(buf, sizeof(buf), "%s*%02X\n", body, cs_xor(body));
  Serial2.print(buf);
  Serial.print("→ "); Serial.print(buf);        // USB 시리얼 모니터에도
}

char line[64];
int n = 0;

void handle(const char *s) {
  const char *star = strchr(s, '*');
  unsigned int got;
  if (!star || sscanf(star + 1, "%2X", &got) != 1 || got != cs_xor(s)) {
    Serial.printf("체크섬 오류: %s\n", s);
    return;
  }
  if (!strncmp(s, "LED=", 4)) {
    digitalWrite(PIN_LED, s[4] == '1');
    Serial.printf("← %s\n", s);
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);    // RX, TX
  pinMode(PIN_LED, OUTPUT);
  dht.begin();
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last >= 1000) {
    last = millis();
    float t = dht.readTemperature(), h = dht.readHumidity();
    char body[32];
    if (isnan(t) || isnan(h)) snprintf(body, sizeof(body), "ERR=DHT");
    else snprintf(body, sizeof(body), "T=%.1f,H=%d", t, (int)h);
    send_line(body);
  }
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\r') continue;
    if (c == '\n') { line[n] = '\0'; if (n) handle(line); n = 0; }
    else if (n < (int)sizeof(line) - 1) line[n++] = c;
  }
}
