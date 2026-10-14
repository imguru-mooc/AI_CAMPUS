/* 07_relay.c — 릴레이 모듈로 다른 회로를 켜고 끄기
 * 배선: 릴레이 모듈 S(IN) → GPIO25 (물리 22), + → 5V, − → GND
 *       접점: COM ↔ NO 사이에 "3V3 → 220Ω → LED → GND" 회로를 끼워 넣는다
 * 동작: 버튼 A(GPIO5) 를 누를 때마다 릴레이 토글 (딸깍 소리)
 */
#include "gpio_common.h"
#include <poll.h>
#include <stdlib.h>

int main(void)
{
    unsigned int rel = 25, btn = 5;
    gpio_install_sigint();
    struct gpiod_line_request *out = gpio_request_output("relay", &rel, 1);
    struct gpiod_line_request *in  = gpio_request_input("relay", &btn, 1, GPIOD_LINE_EDGE_FALLING, 20000);
    if (!out || !in) return EXIT_FAILURE;

    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(4);
    struct pollfd pfd = { .fd = gpiod_line_request_get_fd(in), .events = POLLIN };
    int on = 0;
    printf("버튼 A 로 릴레이 토글 — Ctrl+C 로 종료\n");
    while (!g_stop) {
        if (poll(&pfd, 1, -1) <= 0) continue;
        gpiod_line_request_read_edge_events(in, buf, 4);
        on = !on;
        gpiod_line_request_set_value(out, rel, on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
        printf("  릴레이 %s\n", on ? "ON  (COM-NO 연결)" : "OFF (COM-NC 연결)");
    }
    gpiod_line_request_set_value(out, rel, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_edge_event_buffer_free(buf);
    gpiod_line_request_release(in);
    gpiod_line_request_release(out);
    return EXIT_SUCCESS;
}
