/* 06_edge_poll.c — 커널 엣지 이벤트를 poll() 로 기다리기
 * 배선: GPIO5 버튼(→GND), GPIO17 LED
 * 동작: 짧게 누르면 LED 토글, 0.5초 이상 길게 누르면 "길게" 출력
 *       기다리는 동안 CPU 를 쓰지 않는다 (top 으로 05 와 비교)
 * 실행: ./06_edge_poll        ./06_edge_poll 0   (디바운스 끄기 → 바운싱 관찰)
 */
#include "gpio_common.h"
#include <poll.h>
#include <stdlib.h>

#define BTN 5
#define LED 17
#define LONG_MS 500

int main(int argc, char **argv)
{
    unsigned long deb_us = (argc > 1) ? strtoul(argv[1], NULL, 10) * 1000 : 10000;
    unsigned int btn = BTN, led = LED;
    gpio_install_sigint();

    struct gpiod_line_request *in  = gpio_request_input("edge_poll", &btn, 1,
                                                        GPIOD_LINE_EDGE_BOTH, deb_us);
    struct gpiod_line_request *out = gpio_request_output("edge_poll", &led, 1);
    if (!in || !out) return EXIT_FAILURE;

    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(16);
    struct pollfd pfd = { .fd = gpiod_line_request_get_fd(in), .events = POLLIN };
    long long t_down = 0;
    int led_on = 0;

    printf("버튼 이벤트 대기 (디바운스 %lu ms) — Ctrl+C 로 종료\n", deb_us / 1000);
    while (!g_stop) {
        int r = poll(&pfd, 1, -1);                  /* 이벤트가 올 때까지 잠든다 */
        if (r < 0) { if (errno == EINTR) continue; perror("poll"); break; }

        int n = gpiod_line_request_read_edge_events(in, buf, 16);
        for (int i = 0; i < n; i++) {
            struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(buf, i);
            long long ts = (long long)gpiod_edge_event_get_timestamp_ns(ev);
            int falling = gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_FALLING_EDGE;

            printf("  %lld.%09lld  %s\n", ts / 1000000000LL, ts % 1000000000LL,
                   falling ? "falling (누름)" : "rising  (뗌)");

            if (falling) {
                t_down = ts;
            } else if (t_down) {
                long ms = (long)((ts - t_down) / 1000000);
                if (ms >= LONG_MS) {
                    printf("    → 길게 %ld ms\n", ms);
                } else {
                    led_on = !led_on;
                    gpiod_line_request_set_value(out, led,
                        led_on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
                    printf("    → 짧게 %ld ms, LED %s\n", ms, led_on ? "ON" : "OFF");
                }
                t_down = 0;
            }
        }
    }

    gpiod_line_request_set_value(out, led, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_edge_event_buffer_free(buf);
    gpiod_line_request_release(in);
    gpiod_line_request_release(out);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
