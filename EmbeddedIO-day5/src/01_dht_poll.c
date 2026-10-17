/* 01_dht_poll.c — DHT11 을 사용자 공간에서 직접 읽어 보기 (실패를 보려고 만든 예제)
 *
 *   DHT11 의 비트: High 26~28 µs = 0, High 70 µs = 1  →  수십 µs 차이를 구분해야 한다
 *   ./01_dht_poll          get_value 폴링 + now_ns() 로 High 폭 재기
 *   ./01_dht_poll -e       엣지 이벤트 + 커널 타임스탬프로 재기 (비교용)
 *   ./01_dht_poll -n 20    시도 횟수 (기본 10, 2초 간격)
 *
 * 배선: DHT11 모듈 S → GPIO4 (물리 7) · + → 3V3 · − → GND
 * ⚠ dtoverlay=dht11 이 켜져 있으면 GPIO4 를 커널이 잡고 있어 busy — 이 예제는 overlay 전에 한다
 */
#include <stdlib.h>
#include <poll.h>
#include "gpio_common.h"

#define DHT_PIN 4
#define MAXE 200

static unsigned int g_pin = DHT_PIN;

/* High 펄스 폭(µs) 목록에서 마지막 40개를 비트로 */
static int decode(const long *hw, int n, int *t, int *h)
{
    if (n < 40) return -(100 + n);                  /* 비트 부족 */
    uint8_t b[5] = { 0 };
    for (int i = 0; i < 40; i++) {
        int bit = hw[n - 40 + i] > 50;              /* 50 µs 보다 길면 1 */
        b[i / 8] = (uint8_t)(b[i / 8] << 1 | bit);
    }
    if ((uint8_t)(b[0] + b[1] + b[2] + b[3]) != b[4]) return -1;   /* 체크섬 */
    *h = b[0]; *t = b[2];
    return 0;
}

static struct gpiod_line_config *cfg(int out, int edge)
{
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, out ? GPIOD_LINE_DIRECTION_OUTPUT : GPIOD_LINE_DIRECTION_INPUT);
    if (out) gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);
    else {
        gpiod_line_settings_set_bias(s, GPIOD_LINE_BIAS_PULL_UP);
        if (edge) gpiod_line_settings_set_edge_detection(s, GPIOD_LINE_EDGE_BOTH);
    }
    struct gpiod_line_config *c = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(c, &g_pin, 1, s);
    gpiod_line_settings_free(s);
    return c;
}

/* 한 번 읽기: 0 성공 · -1 체크섬 · -1xx 비트 부족 */
static int read_once(struct gpiod_line_request *req, int use_edge, struct gpiod_edge_event_buffer *buf, int *t, int *h)
{
    struct gpiod_line_config *out = cfg(1, 0), *in = cfg(0, use_edge);
    gpiod_line_request_reconfigure_lines(req, out);         /* 시작 신호: Low 20 ms */
    sleep_ms(20);
    gpiod_line_request_reconfigure_lines(req, in);          /* 놓기 → 센서가 응답 */
    long hw[MAXE]; int n = 0;

    if (!use_edge) {                                        /* ① 폴링 */
        long long t0 = now_ns(), rise = 0;
        int last = 1;
        while (now_ns() - t0 < 8000000LL && n < MAXE) {     /* 8 ms 동안 계속 읽기 */
            int v = gpiod_line_request_get_value(req, g_pin) == GPIOD_LINE_VALUE_ACTIVE;
            if (v != last) {
                long long now = now_ns();                   /* ← 읽은 "순간" 이 아니라 지금 */
                if (v) rise = now;
                else if (rise) hw[n++] = (long)((now - rise) / 1000);
                last = v;
            }
        }
    } else {                                                /* ② 엣지 이벤트 */
        struct pollfd p = { .fd = gpiod_line_request_get_fd(req), .events = POLLIN };
        long long rise = 0;
        while (n < MAXE && poll(&p, 1, 8) > 0) {
            int m = gpiod_line_request_read_edge_events(req, buf, 64);
            for (int i = 0; i < m && n < MAXE; i++) {
                struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(buf, i);
                long long ts = (long long)gpiod_edge_event_get_timestamp_ns(ev);
                if (gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_RISING_EDGE) rise = ts;
                else if (rise) hw[n++] = (long)((ts - rise) / 1000);
            }
        }
    }
    gpiod_line_config_free(out);
    gpiod_line_config_free(in);
    return decode(hw, n, t, h);
}

int main(int argc, char **argv)
{
    int use_edge = 0, tries = 10;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-e")) use_edge = 1;
        else if (!strcmp(argv[i], "-n") && i + 1 < argc) tries = atoi(argv[++i]);
    }
    gpio_install_sigint();
    /* 요청을 직접 만든다: 엣지 80개가 한꺼번에 오므로 커널 이벤트 버퍼를 넉넉히 (기본 = 라인당 16개) */
    struct gpiod_chip *chip = gpiod_chip_open(GPIO_CHIP);
    struct gpiod_line_config *lc = cfg(0, 0);
    struct gpiod_request_config *rc = gpiod_request_config_new();
    gpiod_request_config_set_consumer(rc, "dht-poll");
    gpiod_request_config_set_event_buffer_size(rc, 256);
    struct gpiod_line_request *req = chip ? gpiod_chip_request_lines(chip, rc, lc) : NULL;
    gpiod_request_config_free(rc);
    gpiod_line_config_free(lc);
    if (chip) gpiod_chip_close(chip);
    if (!req) perror("GPIO4 요청 (dht11 overlay 가 켜져 있으면 busy)");
    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(64);
    if (!req || !buf) return EXIT_FAILURE;

    printf("DHT11 사용자 공간 읽기 (%s) × %d — Ctrl+C 로 종료\n", use_edge ? "엣지 타임스탬프" : "폴링", tries);
    int ok = 0, done = 0;
    for (; done < tries && !g_stop; done++) {
        int t, h, r = read_once(req, use_edge, buf, &t, &h);
        if (r == 0) { ok++; printf("%2d  OK      %d °C  %d %%\n", done + 1, t, h); }
        else if (r == -1) printf("%2d  실패    체크섬 오류 (비트 하나 이상 잘못 읽음)\n", done + 1);
        else printf("%2d  실패    비트 부족 (%d / 40)\n", done + 1, -r - 100);
        sleep_ms(2000);                                     /* DHT11 은 1초에 한 번 이하 */
    }
    printf("성공 %d / %d\n", ok, done);
    gpiod_edge_event_buffer_free(buf);
    gpiod_line_request_release(req);
    return EXIT_SUCCESS;
}
