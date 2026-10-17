/* eio_eval_template.c — 2차 평가 · Embedded I/O 제어 실습 (정규 평가)
 *
 * 환경 : Raspberry Pi 4 · Raspberry Pi OS Lite 64-bit (Trixie) · libgpiod 2.x · PuTTY 접속
 * 배선 : (Day 2 배선과 같은 자리 — 부품 4개 사용)
 *        GPIO17 (물리 11) → 220Ω → LED 빨강(+, 긴 다리) → GND
 *        GPIO22 (물리 15) → 220Ω → LED 초록(+, 긴 다리) → GND
 *        GPIO13 (물리 33) → 액티브 부저(+) → GND       (뒷면이 막힌 부저)
 *        GPIO5  (물리 29) → 버튼 → GND                (외부 저항 없음, 내부 pull-up 사용)
 * 빌드 : gcc -Wall -O2 -o eio_eval eio_eval_template.c -lgpiod
 *        (또는 수업 공통 Makefile 이 있는 폴더에서  make eio_eval_template)
 * 실행 : ./eio_eval        — 1~6단계가 순서대로 진행된다. 화면 안내에 따라 버튼을 누른다.
 *
 * 규칙
 *  - 빈칸 ______ 8곳과 TODO 3곳만 채운다. [N단계] 결과 / 기대값 printf 는 고치지 않는다.
 *  - 결과와 기대값이 같고, LED · 부저가 설명대로 동작하면 그 단계는 완료.
 *  - "Device or resource busy" 가 나오면 다른 프로그램(gpioset 등)이 라인을 쥐고 있는 것 → 그 프로그램을 끝낸다.
 *
 *  단계  주제                                          Day   형태
 *   1    출력 라인 3개 요청 (chip → settings → … → request)   2   빈칸 4
 *   2    LED 5번 깜빡이기 + 부저 (set_value)            2   빈칸 1
 *   3    입력 라인 요청 · bias · 값 읽기                2   빈칸 3
 *   4    폴링 — 누르는 동안 LED ON · 눌림 횟수           2   TODO 1
 *   5    엣지 이벤트 — 누를 때마다 LED 토글              2   TODO 2
 *   6    반응 시간 — 커널 타임스탬프로 ms 재기           2   TODO 3
 */
#include <gpiod.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define CHIP_PATH "/dev/gpiochip0"
#define LED  17          /* 빨강 */
#define LED2 22          /* 초록 */
#define BUZ  13          /* 액티브 부저 — 1 이면 스스로 소리를 낸다 */
#define BTN  5

static const unsigned int OUTS[3] = { LED, LED2, BUZ };

/* ── 제공 도우미 (고치지 않는다) ── */
static void sleep_ms(long ms)
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

static long long now_ns(void)              /* 엣지 이벤트 타임스탬프와 같은 시계 */
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static void set_line(struct gpiod_line_request *req, unsigned int line, int on)
{
    gpiod_line_request_set_value(req, line,
        on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

/* settings 하나로 라인 1개를 요청한다. 실패하면 NULL */
static struct gpiod_line_request *request_one(unsigned int offset, const char *consumer,
                                              struct gpiod_line_settings *s)
{
    struct gpiod_chip *chip = gpiod_chip_open(CHIP_PATH);
    if (!chip) { perror("gpiod_chip_open"); return NULL; }
    struct gpiod_line_config *lc = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(lc, &offset, 1, s);
    struct gpiod_request_config *rc = gpiod_request_config_new();
    gpiod_request_config_set_consumer(rc, consumer);
    struct gpiod_line_request *req = gpiod_chip_request_lines(chip, rc, lc);
    if (!req) fprintf(stderr, "%s: 라인 요청 실패: %s\n", consumer, strerror(errno));
    gpiod_request_config_free(rc);
    gpiod_line_config_free(lc);
    gpiod_chip_close(chip);                     /* 요청은 칩을 닫아도 유지된다 */
    return req;
}

/* ───────────── 1단계: 출력 라인 3개 요청 ─────────────
 * libgpiod v2 의 객체 흐름을 직접 쓴다: chip → settings → line_config → request_config → request
 * LED 빨강 · LED 초록 · 부저 세 라인을 한 요청으로 묶는다. */
static struct gpiod_line_request *step1_request_outputs(void)
{
    struct gpiod_chip *chip = gpiod_chip_open(______);                 /* ① 40핀 헤더 GPIO 를 가진 칩의 장치 파일 경로 */
    if (!chip) { perror("gpiod_chip_open"); return NULL; }

    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, ______);                      /* ② 출력 방향 상수 */
    gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);

    struct gpiod_line_config *lc = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(lc, OUTS, ______, s);          /* ③ 한 번에 요청할 라인 개수 */

    struct gpiod_request_config *rc = gpiod_request_config_new();
    gpiod_request_config_set_consumer(rc, "eval_out");

    struct gpiod_line_request *req = ______(chip, rc, lc);             /* ④ 커널에 라인을 요청하는 함수 */

    gpiod_request_config_free(rc);
    gpiod_line_config_free(lc);
    gpiod_line_settings_free(s);
    gpiod_chip_close(chip);

    printf("[1단계] 결과 : 요청 %s\n", req ? "성공" : "실패");
    printf("[1단계] 기대값: 요청 성공\n\n");
    return req;
}

/* ───────────── 2단계: LED 5번 깜빡이기 + 부저 ─────────────
 * 빨강 LED 를 5번 깜빡이고, 마지막에 초록 LED 와 부저를 0.2초 켠다. */
static void step2_blink(struct gpiod_line_request *out)
{
    int count = 0;
    for (int i = 0; i < 5; i++) {
        gpiod_line_request_set_value(out, LED, ______);                /* ① 빨강 LED 켜기 (논리값 1) */
        sleep_ms(300);
        gpiod_line_request_set_value(out, LED, GPIOD_LINE_VALUE_INACTIVE);
        sleep_ms(300);
        count++;
    }
    set_line(out, LED2, 1); set_line(out, BUZ, 1);    /* 끝 알림: 초록 + 삐 */
    sleep_ms(200);
    set_line(out, LED2, 0); set_line(out, BUZ, 0);

    printf("[2단계] 결과 : 깜빡임 %d회\n", count);
    printf("[2단계] 기대값: 깜빡임 5회 (빨강 0.3초 간격 5번 → 초록 LED 와 부저 '삐' 한 번)\n\n");
}

/* ───────────── 3단계: 입력 라인 요청과 bias ─────────────
 * 버튼은 GND 쪽에 달려 있다 → 평소 1 을 만들어 주는 bias 가 필요하다. */
static struct gpiod_line_request *request_button(enum gpiod_line_edge edge, const char *consumer)
{
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, ______);                      /* ① 입력 방향 상수 */
    gpiod_line_settings_set_bias(s, ______);                           /* ② 내부 pull-up 상수 */
    gpiod_line_settings_set_edge_detection(s, edge);
    if (edge != GPIOD_LINE_EDGE_NONE)
        gpiod_line_settings_set_debounce_period_us(s, 10000);          /* 커널 디바운스 10 ms */
    struct gpiod_line_request *req = request_one(BTN, consumer, s);
    gpiod_line_settings_free(s);
    return req;
}

static void step3_read(void)
{
    struct gpiod_line_request *btn = request_button(GPIOD_LINE_EDGE_NONE, "eval_btn");
    if (!btn) return;

    int idle = gpiod_line_request_get_value(btn, BTN);
    printf("  버튼을 3초 안에 눌러서 누른 채로 있으세요...\n");
    sleep_ms(3000);
    int pressed = ______(btn, BTN);                                    /* ③ 라인 값 하나 읽기 */
    printf("  이제 버튼에서 손을 떼세요.\n");
    sleep_ms(1000);

    gpiod_line_request_release(btn);
    printf("[3단계] 결과 : 안 누름=%d, 누름=%d\n", idle, pressed);
    printf("[3단계] 기대값: 안 누름=1, 누름=0\n\n");
}

/* ───────────── 4단계: 폴링 ─────────────
 * 10초 동안 20 ms 마다 버튼을 읽어, 누르는 동안만 빨강 LED 를 켜고 눌린 횟수를 센다.
 * (20 ms 간격으로 읽는 것 자체가 간단한 디바운스 역할을 한다) */
static void step4_poll(struct gpiod_line_request *out)
{
    struct gpiod_line_request *btn = request_button(GPIOD_LINE_EDGE_NONE, "eval_poll");
    if (!btn) return;

    printf("  10초 동안 버튼을 3번 누르세요 (누르는 동안만 LED ON)\n");
    int last = 1, presses = 0;
    long long t0 = now_ns();
    while (now_ns() - t0 < 10000000000LL) {
        int v = gpiod_line_request_get_value(btn, BTN);   /* 1 = 안 누름, 0 = 누름 */
        /* TODO 1:
         *   ① v 가 0 이면 빨강 LED 를 켜고, 1 이면 끈다 (set_line 사용 가능)
         *   ② last 가 1 이고 v 가 0 인 순간(눌리는 순간)에만 presses 를 1 늘리고
         *      "  눌림 #presses" 를 출력한다
         *   ③ last 를 v 로 갱신한다                                              */

        sleep_ms(20);
    }
    set_line(out, LED, 0);
    gpiod_line_request_release(btn);

    printf("[4단계] 결과 : 눌림 %d회\n", presses);
    printf("[4단계] 기대값: 눌림 3회 (3번 눌렀을 때 · 누르는 동안만 LED 가 켜짐)\n\n");
}

/* ───────────── 5단계: 엣지 이벤트 ─────────────
 * falling 엣지(누르는 순간)만 받는다. 누를 때마다 빨강 LED 를 토글하고, 3번 받으면 끝낸다.
 * poll() 이 기다리는 동안 프로세스는 잠들어 CPU 를 쓰지 않는다. */
static void step5_edge(struct gpiod_line_request *out)
{
    struct gpiod_line_request *btn = request_button(GPIOD_LINE_EDGE_FALLING, "eval_edge");
    if (!btn) return;

    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(8);
    struct pollfd pfd = { .fd = gpiod_line_request_get_fd(btn), .events = POLLIN };
    int events = 0, led_on = 0;

    printf("  버튼을 3번 누르세요 (누를 때마다 LED 토글, 20초 제한)\n");
    while (events < 3) {
        int r = poll(&pfd, 1, 20000);                     /* 이벤트가 올 때까지 잠든다 */
        if (r <= 0) { printf("  시간 초과\n"); break; }

        int n = gpiod_line_request_read_edge_events(btn, buf, 8);
        for (int i = 0; i < n && events < 3; i++) {
            /* TODO 2:
             *   ① events 를 1 늘린다
             *   ② led_on 을 반전하고 빨강 LED 에 출력한다
             *   ③ "  이벤트 #events  LED ON/OFF" 를 출력한다                       */

        }
    }
    set_line(out, LED, 0);
    gpiod_edge_event_buffer_free(buf);
    gpiod_line_request_release(btn);

    printf("[5단계] 결과 : 이벤트 %d회, 마지막 LED=%s\n", events, led_on ? "ON" : "OFF");
    printf("[5단계] 기대값: 이벤트 3회, 마지막 LED=ON (홀수 번 토글)\n\n");
}

/* ───────────── 6단계: 반응 시간 재기 ─────────────
 * 2초 뒤 초록 LED 와 부저가 켜지면 최대한 빨리 버튼을 누른다.
 * 반응 시간 = (버튼 엣지의 커널 타임스탬프) − (초록 LED 를 켠 시각). 같은 CLOCK_MONOTONIC 시계라 바로 뺄 수 있다. */
static void step6_reaction(struct gpiod_line_request *out)
{
    struct gpiod_line_request *btn = request_button(GPIOD_LINE_EDGE_FALLING, "eval_react");
    if (!btn) return;

    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(8);
    struct pollfd pfd = { .fd = gpiod_line_request_get_fd(btn), .events = POLLIN };
    long react_ms = -1;

    printf("  준비... 초록 LED 와 부저가 켜지면 버튼을 누르세요\n");
    sleep_ms(2000);
    while (poll(&pfd, 1, 0) > 0)                          /* 켜지기 전에 누른 이벤트는 버린다 (drain) */
        gpiod_line_request_read_edge_events(btn, buf, 8);
    set_line(out, LED2, 1);
    set_line(out, BUZ, 1);
    long long t_on = now_ns();

    if (poll(&pfd, 1, 5000) > 0) {
        int n = gpiod_line_request_read_edge_events(btn, buf, 8);
        if (n > 0) {
            struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(buf, 0);
            long long ts = (long long)gpiod_edge_event_get_timestamp_ns(ev);   /* 커널 타임스탬프(ns) */
            /* TODO 3:
             *   ① react_ms 에 (ts − t_on) 을 ms 로 바꾼 값을 넣는다 (ns → ms 는 1000000 으로 나눔)
             *   ② 초록 LED 와 부저를 끈다
             *   ③ "  반응 시간: react_ms ms" 를 출력한다                              */

        }
    }
    set_line(out, LED2, 0);
    set_line(out, BUZ, 0);
    gpiod_edge_event_buffer_free(buf);
    gpiod_line_request_release(btn);

    printf("[6단계] 결과 : 반응 시간 측정 %s\n", (react_ms > 0 && react_ms < 5000) ? "성공" : "실패");
    printf("[6단계] 기대값: 반응 시간 측정 성공 (보통 150~600 ms, 누르는 순간 초록 LED · 부저 꺼짐)\n\n");
}

int main(void)
{
    struct gpiod_line_request *out = step1_request_outputs();
    if (!out) return EXIT_FAILURE;

    step2_blink(out);
    step3_read();
    step4_poll(out);
    step5_edge(out);
    step6_reaction(out);

    enum gpiod_line_value off[3] = { GPIOD_LINE_VALUE_INACTIVE, GPIOD_LINE_VALUE_INACTIVE,
                                     GPIOD_LINE_VALUE_INACTIVE };
    gpiod_line_request_set_values(out, off);
    gpiod_line_request_release(out);            /* 요청한 라인은 반드시 반납 */
    printf("라인 반납 완료\n");
    return EXIT_SUCCESS;
}
