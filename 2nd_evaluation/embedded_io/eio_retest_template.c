/* eio_retest_template.c — 2차 평가 · Embedded I/O 제어 실습 (재평가용 대체 과제)
 *
 * 환경 : Raspberry Pi 4 · Raspberry Pi OS Lite 64-bit (Trixie) · libgpiod 2.x · PuTTY 접속
 * 배선 : (Day 2 신호등 배선 + 버튼 2개 + 부저 — 부품 6개 사용)
 *        GPIO17 (물리 11) → 220Ω → LED 빨강 → GND
 *        GPIO27 (물리 13) → 220Ω → LED 노랑 → GND
 *        GPIO22 (물리 15) → 220Ω → LED 초록 → GND
 *        GPIO5  (물리 29) → 버튼 A → GND         (내부 pull-up)
 *        GPIO6  (물리 31) → 버튼 B → GND         (내부 pull-up)
 *        GPIO13 (물리 33) → 액티브 부저(+) → GND
 * 빌드 : gcc -Wall -O2 -o eio_retest eio_retest_template.c -lgpiod
 * 실행 : ./eio_retest      — 1~6단계가 순서대로 진행된다.
 *
 * 6단계 · 빈칸 ______ 8곳 + TODO 3곳 · 난이도 3구간
 *   [쉬움   1·2·6] 빈칸 채우기 (각 2곳)
 *   [보통   3·4  ] 빈칸 2곳 + TODO 1 (두 줄) · TODO 2 (한 줄)
 *   [어려움 5    ] TODO 3 (반복문)
 * [N단계] 결과 / 기대값 printf 는 고치지 않는다. 두 줄이 같고 LED 가 설명대로 동작하면 완료.
 */
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define CHIP_PATH "/dev/gpiochip0"
#define BTN_A 5
#define BTN_B 6
#define BUZ   13         /* 액티브 부저 — 1 이면 스스로 소리를 낸다 */

static const unsigned int BTNS[2] = { BTN_A, BTN_B };

static const unsigned int LEDS[3] = { 17, 27, 22 };      /* 빨강 · 노랑 · 초록 */

/* ── 제공 도우미 (고치지 않는다) ── */
static void sleep_ms(long ms)
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

static long long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static struct gpiod_line_request *request_lines(const unsigned int *offsets, size_t n,
                                                const char *consumer,
                                                struct gpiod_line_settings *s)
{
    struct gpiod_chip *chip = gpiod_chip_open(CHIP_PATH);
    if (!chip) { perror("gpiod_chip_open"); return NULL; }
    struct gpiod_line_config *lc = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(lc, offsets, n, s);
    struct gpiod_request_config *rc = gpiod_request_config_new();
    gpiod_request_config_set_consumer(rc, consumer);
    struct gpiod_line_request *req = gpiod_chip_request_lines(chip, rc, lc);
    if (!req) fprintf(stderr, "%s: 라인 요청 실패: %s\n", consumer, strerror(errno));
    gpiod_request_config_free(rc);
    gpiod_line_config_free(lc);
    gpiod_chip_close(chip);
    return req;
}

/* 0/1 세 개를 LED 세 개에 한 번에 출력 */
static void show3(struct gpiod_line_request *req, int r, int y, int g)
{
    enum gpiod_line_value v[3] = {
        r ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
        y ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
        g ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
    };
    gpiod_line_request_set_values(req, v);
}

/* ───────────── [쉬움] 1단계: 출력 라인 3개를 한 요청으로 ───────────── */
static struct gpiod_line_request *step1_request(void)
{
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, ______);                     /* ① 출력 방향 상수 */
    gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);
    struct gpiod_line_request *req = request_lines(LEDS, ______, "retest_led", s);   /* ② 라인 개수 */
    gpiod_line_settings_free(s);

    if (req) { show3(req, 1, 1, 1); sleep_ms(1000); show3(req, 0, 0, 0); }
    printf("[1단계] 결과 : 요청 %s\n", req ? "성공" : "실패");
    printf("[1단계] 기대값: 요청 성공 (LED 3개가 1초 동안 함께 켜졌다 꺼짐)\n\n");
    return req;
}

/* ───────────── [쉬움] 2단계: 상태표로 신호등 한 바퀴 ─────────────
 * 상태표의 각 행 = { 빨강, 노랑, 초록, 유지 시간(ms) } */
static const struct { int r, y, g, ms; const char *name; } TABLE[] = {
    { 0, 0, 1, 2000, "초록" },
    { 0, 1, 0, 1000, "노랑" },
    { 1, 0, 0, 2000, "빨강" },
};
#define N_TABLE (int)(sizeof TABLE / sizeof TABLE[0])

static void step2_traffic(struct gpiod_line_request *req)
{
    int shown = 0;
    for (int i = 0; i < ______; i++) {                                 /* ① 상태 개수 매크로 */
        show3(req, TABLE[i].r, TABLE[i].y, ______);                    /* ② i 번째 상태의 초록 값 */
        printf("  %-4s R=%d Y=%d G=%d\n", TABLE[i].name, TABLE[i].r, TABLE[i].y, TABLE[i].g);
        sleep_ms(TABLE[i].ms);
        shown++;
    }
    show3(req, 0, 0, 0);
    printf("[2단계] 결과 : 상태 %d개 재생\n", shown);
    printf("[2단계] 기대값: 상태 3개 재생 (초록 2초 → 노랑 1초 → 빨강 2초)\n\n");
}

/* ───────────── [보통] 3단계: 버튼 2개를 한 요청으로 · 값 읽기 ───────────── */
static struct gpiod_line_request *request_buttons(void)
{
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(s, ______);                           /* ① 내부 pull-up 상수 */
    struct gpiod_line_request *req = request_lines(BTNS, ______, "retest_btn", s);   /* ② 라인 개수 */
    gpiod_line_settings_free(s);
    return req;
}

static void step3_read(struct gpiod_line_request *btn)
{
    int idle_a = 0, idle_b = 0;
    /* TODO 1: 버튼 A(BTN_A) 값을 idle_a 에, 버튼 B(BTN_B) 값을 idle_b 에 읽어 넣는다 (두 줄) */

    printf("[3단계] 결과 : 안 누름 값 A=%d, B=%d\n", idle_a, idle_b);
    printf("[3단계] 기대값: 안 누름 값 A=1, B=1\n\n");
}

/* ───────────── [보통] 4단계: 버튼 A 를 누를 때마다 다음 LED 하나만 ─────────────
 * 처음에는 빨강(0번)만 켜져 있다. 버튼 A 를 누를 때마다 빨강 → 노랑 → 초록 → 빨강 … 순서로 하나만 켠다.
 * 4번 누르면 끝낸다. */
static void step4_next(struct gpiod_line_request *leds, struct gpiod_line_request *btn)
{
    int idx = 0, last = 1, presses = 0;
    show3(leds, 1, 0, 0);
    printf("  버튼 A 를 4번 누르세요 (30초 제한)\n");
    long long t0 = now_ms();
    while (presses < 4 && now_ms() - t0 < 30000) {
        int v = gpiod_line_request_get_value(btn, BTN_A);
        if (last == 1 && v == 0) {
            presses++;
            /* TODO 2: idx 를 다음 번호로 바꾼다. 2 다음은 0 으로 돌아가야 한다 (% 사용, 한 줄) */

            show3(leds, idx == 0, idx == 1, idx == 2);
            printf("  눌림 #%d → LED %d번\n", presses, idx);
        }
        last = v;
        sleep_ms(20);                                   /* 간단한 디바운스 */
    }
    show3(leds, 0, 0, 0);
    printf("[4단계] 결과 : 눌림 %d회, 마지막 idx=%d\n", presses, idx);
    printf("[4단계] 기대값: 눌림 4회, 마지막 idx=1 (1 → 2 → 0 → 1 순서로 출력)\n\n");
}

/* ───────────── [어려움] 5단계: 3비트 이진 카운터 ─────────────
 * 0 ~ 7 을 LED 3개에 2진수로 표시한다. bit0 → 빨강(17), bit1 → 노랑(27), bit2 → 초록(22). */
static void step5_binary(struct gpiod_line_request *leds)
{
    int lit_total = 0;                                  /* 0~7 동안 켜진 LED 수의 합 */
    for (int n = 0; n < 8; n++) {
        int bit[3] = { 0, 0, 0 };
        /* TODO 3:
         *   ① b = 0, 1, 2 에 대해 bit[b] 에 n 의 b 번째 비트 값(0 또는 1)을 넣는다 — (n >> b) & 1
         *   ② 켜진 비트 수를 lit_total 에 더한다                                         */

        show3(leds, bit[0], bit[1], bit[2]);
        printf("  n=%d → 초록 노랑 빨강 = %d %d %d\n", n, bit[2], bit[1], bit[0]);
        sleep_ms(700);
    }
    show3(leds, 0, 0, 0);
    printf("[5단계] 결과 : 켜진 LED 합계=%d\n", lit_total);
    printf("[5단계] 기대값: 켜진 LED 합계=12 (n=5 일 때 빨강 · 초록이 켜짐)\n\n");
}

/* ───────────── [쉬움] 6단계: 버튼 B → 부저 ─────────────
 * 버튼 B 를 누를 때마다 부저를 0.1초 울린다. 3번 울리면 끝낸다. */
static void step6_buzzer(struct gpiod_line_request *btn)
{
    unsigned int off = BUZ;
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);
    struct gpiod_line_request *buz = request_lines(&off, 1, "retest_buz", s);
    gpiod_line_settings_free(s);
    if (!buz) return;

    int last = 1, beeps = 0;
    printf("  버튼 B 를 3번 누르세요 (누를 때마다 '삐', 30초 제한)\n");
    long long t0 = now_ms();
    while (beeps < 3 && now_ms() - t0 < 30000) {
        int v = gpiod_line_request_get_value(btn, ______);             /* ① 버튼 B 의 라인 번호 */
        if (last == 1 && v == 0) {
            gpiod_line_request_set_value(buz, BUZ, ______);            /* ② 부저 켜기 (논리값 1) */
            sleep_ms(100);
            gpiod_line_request_set_value(buz, BUZ, GPIOD_LINE_VALUE_INACTIVE);
            printf("  삐 #%d\n", ++beeps);
        }
        last = v;
        sleep_ms(20);
    }
    gpiod_line_request_release(buz);
    printf("[6단계] 결과 : 삐 %d회\n", beeps);
    printf("[6단계] 기대값: 삐 3회 (버튼 A 를 눌러도 울리지 않음)\n\n");
}

int main(void)
{
    struct gpiod_line_request *leds = step1_request();
    if (!leds) return EXIT_FAILURE;
    step2_traffic(leds);

    struct gpiod_line_request *btn = request_buttons();
    if (btn) {
        step3_read(btn);
        step4_next(leds, btn);
    }
    step5_binary(leds);
    if (btn) {
        step6_buzzer(btn);
        gpiod_line_request_release(btn);
    }

    gpiod_line_request_release(leds);
    printf("라인 반납 완료\n");
    return EXIT_SUCCESS;
}
