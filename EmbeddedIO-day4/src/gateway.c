/* gateway.c — Pi ↔ ESP32 센서 게이트웨이 (Day 4 미니 프로젝트)
 *
 *   ESP32 → Pi : "T=23.0,H=45*2E\n"  1초마다 (DHT11)      * 뒤 두 자리 = XOR 체크섬(16진)
 *   Pi → ESP32 : "LED=1*41\n"        어두우면 1, 밝으면 0 (바뀔 때 + 10초마다)
 *   4자리 7-Segment: 온도 "23.0" ↔ 습도 "H 45" 를 2초씩 번갈아
 *   3초 동안 정상 프레임이 없으면 "----"
 *
 *   ./gateway          ESP32 와 연결
 *   ./gateway -s       ESP32 없이: TX↔RX 루프백 선 + Pi 가 가짜 프레임을 보냄
 *   ./gateway -k       ESP32 없이: 키보드로 프레임 입력 (체크섬 없이 입력해도 받음)
 *   ./gateway -f       7-Segment 표시 쓰레드 SCHED_FIFO
 */
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include "seg4.h"
#include "pcf8591.h"
#include "uart.h"

#define TIMEOUT_NS   3000000000LL
#define DARK_ON   30                        /* 조도 30% 아래면 LED=1 */
#define DARK_OFF  40                        /* 40% 위로 올라가야 LED=0 */

typedef struct {
    double t; int h;
    long long at;                           /* 마지막 정상 프레임 시각 */
    int ok, bad_cs, bad_fmt;
} reading_t;

/* 한 줄 처리: 0 정상 · -1 체크섬 · -2 형식 */
static int handle_line(const char *line, int need_cs, reading_t *r)
{
    int c = cs_check(line);
    if (c == 0 || (c < 0 && need_cs)) { r->bad_cs++; return -1; }
    double t; int h;
    if (sscanf(line, "T=%lf,H=%d", &t, &h) != 2 || t < -20 || t > 80 || h < 0 || h > 100) {
        r->bad_fmt++; return -2;
    }
    r->t = t; r->h = h; r->at = now_ns(); r->ok++;
    return 0;
}

int main(int argc, char **argv)
{
    int sim = 0, kbd = 0, fifo = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-s")) sim = 1;
        else if (!strcmp(argv[i], "-k")) kbd = 1;
        else if (!strcmp(argv[i], "-f")) fifo = 1;
    }
    gpio_install_sigint();
    int uart = kbd ? -1 : uart_open(UART_DEV, 115200);
    int pcf  = pcf_open("/dev/i2c-1", PCF_ADDR);
    int spi  = spi_open(SPI_DEV, 1000000);
    if ((!kbd && uart < 0) || pcf < 0 || spi < 0) return EXIT_FAILURE;
    seg4_t seg;
    if (seg4_start(&seg, spi, fifo) < 0) return EXIT_FAILURE;
    seg4_text(&seg, "----");

    /* epoll: UART(또는 키보드) + 1초 타이머 */
    int src = kbd ? STDIN_FILENO : uart;
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    struct itimerspec its = { { 1, 0 }, { 1, 0 } };
    timerfd_settime(tfd, 0, &its, NULL);
    int ep = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event e = { .events = EPOLLIN };
    e.data.fd = src; epoll_ctl(ep, EPOLL_CTL_ADD, src, &e);
    e.data.fd = tfd; epoll_ctl(ep, EPOLL_CTL_ADD, tfd, &e);

    printf("게이트웨이 시작%s — Ctrl+C 로 종료\n", sim ? " (루프백 모의)" : kbd ? " (키보드 입력)" : "");
    reading_t r = { 0 };
    linebuf_t lb = { 0 };
    int dark = -1, tick = 0, timed_out = 1;
    while (!g_stop) {
        struct epoll_event ev[2];
        int n = epoll_wait(ep, ev, 2, -1);
        if (n < 0) break;
        for (int i = 0; i < n; i++) {
            if (ev[i].data.fd == src) {
                char buf[64];
                ssize_t m = read(src, buf, sizeof buf);
                if (m <= 0) { if (kbd) g_stop = 1; continue; }
                for (ssize_t k = 0; k < m; k++) {
                    if (!lb_feed(&lb, buf[k]) || !lb.line[0]) continue;
                    if (!strncmp(lb.line, "LED=", 4)) continue;     /* 루프백으로 돌아온 내 명령 */
                    if (!strncmp(lb.line, "ERR=", 4)) { printf("ESP32 오류: %s\n", lb.line); continue; }
                    int rc = handle_line(lb.line, !kbd, &r);
                    if (rc == 0) {
                        if (timed_out) printf("수신 재개\n");
                        timed_out = 0;
                        printf("T %.1f °C  H %d %%\n", r.t, r.h);
                    } else printf("버림(%s): %s\n", rc == -1 ? "체크섬" : "형식", lb.line);
                }
            } else {                                                /* 1초 타이머 */
                uint64_t exp; if (read(tfd, &exp, sizeof exp) < 0) continue;
                tick++;
                if (sim) {                                          /* 가짜 ESP32 프레임 */
                    char body[32];
                    snprintf(body, sizeof body, "T=%.1f,H=%d", 22.0 + (tick % 20) / 10.0, 40 + tick % 7);
                    uart_send_line(uart, body);
                }
                /* 조도 → LED 명령 (히스테리시스) */
                int v = pcf_read(pcf, 0), light = v < 0 ? -1 : v * 100 / 255;
                int want = dark;
                if (light >= 0) {
                    if (dark != 1 && light < DARK_ON) want = 1;
                    else if (dark != 0 && light > DARK_OFF) want = 0;
                }
                if (want != dark || (tick % 10 == 0 && dark >= 0)) {
                    dark = want;
                    if (uart >= 0) uart_send_line(uart, dark ? "LED=1" : "LED=0");
                    printf("조도 %d%% → LED=%d\n", light, dark);
                }
                /* 표시 · 타임아웃 */
                if (!r.at || now_ns() - r.at > TIMEOUT_NS) {
                    if (!timed_out) printf("3초 동안 정상 프레임 없음\n");
                    timed_out = 1;
                    seg4_text(&seg, "----");
                } else if ((tick / 2) % 2 == 0) {
                    char s[8]; snprintf(s, sizeof s, "%.1f", r.t); seg4_text(&seg, s);
                } else {
                    char s[8]; snprintf(s, sizeof s, "H%3d", r.h); seg4_text(&seg, s);
                }
            }
        }
    }
    printf("\n정상 %d · 체크섬 오류 %d · 형식 오류 %d\n", r.ok, r.bad_cs, r.bad_fmt);
    seg4_stop(&seg);
    close(ep); close(tfd); close(spi); close(pcf);
    if (uart >= 0) close(uart);
    printf("종료\n");
    return EXIT_SUCCESS;
}
