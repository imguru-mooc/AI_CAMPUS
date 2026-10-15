/* 08_uart_term.c — /dev/serial0 간이 터미널 (키보드 ↔ UART), epoll 한 루프
 *
 *   ./08_uart_term              입력한 줄을 보내고, 받은 줄을 출력
 *   ./08_uart_term -t           루프백 시험: 1초마다 "ping N" 을 보내고 돌아오는 시간 측정
 *   ./08_uart_term -b 9600      속도 바꾸기 (상대와 다르면 글자가 깨진다)
 *
 * 루프백: GPIO14(TXD, 물리 8) ↔ GPIO15(RXD, 물리 10) 를 선 하나로 잇는다
 * ESP32 : Pi TXD → ESP32 RX2(16) · Pi RXD ← ESP32 TX2(17) · GND ↔ GND
 */
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include "gpio_common.h"
#include "uart.h"

int main(int argc, char **argv)
{
    int baud = 115200, test = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) test = 1;
        else if (!strcmp(argv[i], "-b") && i + 1 < argc) baud = atoi(argv[++i]);
    }
    gpio_install_sigint();
    int fd = uart_open(UART_DEV, baud);
    if (fd < 0) return EXIT_FAILURE;

    int ep = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event e = { .events = EPOLLIN };
    e.data.fd = fd;            epoll_ctl(ep, EPOLL_CTL_ADD, fd, &e);
    e.data.fd = STDIN_FILENO;  epoll_ctl(ep, EPOLL_CTL_ADD, STDIN_FILENO, &e);
    int tfd = -1;
    if (test) {
        tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
        struct itimerspec its = { { 1, 0 }, { 1, 0 } };
        timerfd_settime(tfd, 0, &its, NULL);
        e.data.fd = tfd; epoll_ctl(ep, EPOLL_CTL_ADD, tfd, &e);
    }
    printf("%s %d 8N1 %s— Ctrl+C 로 종료\n", UART_DEV, baud, test ? "루프백 시험 " : "");

    linebuf_t lb = { 0 };
    long long sent_at = 0;
    int seq = 0, lost = 0, got = 0;
    while (!g_stop) {
        struct epoll_event ev[3];
        int n = epoll_wait(ep, ev, 3, -1);
        if (n < 0) break;                               /* Ctrl+C → EINTR */
        for (int i = 0; i < n; i++) {
            int f = ev[i].data.fd;
            if (f == fd) {                              /* UART 에서 받은 바이트 */
                char buf[64];
                ssize_t r = read(fd, buf, sizeof buf);
                for (ssize_t k = 0; k < r; k++)
                    if (lb_feed(&lb, buf[k])) {
                        if (test && sent_at) {
                            got++;
                            printf("← %-10s  %.2f ms\n", lb.line, (now_ns() - sent_at) / 1e6);
                            sent_at = 0;
                        } else printf("← %s\n", lb.line);
                    }
            } else if (f == STDIN_FILENO) {             /* 키보드 한 줄 → UART */
                char line[128];
                if (!fgets(line, sizeof line, stdin)) { g_stop = 1; break; }
                if (write(fd, line, strlen(line)) < 0) perror("write");
            } else if (f == tfd) {                      /* 1초 타이머 → ping */
                uint64_t exp; if (read(tfd, &exp, sizeof exp) < 0) continue;
                if (sent_at) { lost++; printf("   (응답 없음 — TX·RX 연결 확인)\n"); }
                char msg[32];
                int m = snprintf(msg, sizeof msg, "ping %d\n", ++seq);
                sent_at = now_ns();
                if (write(fd, msg, m) != m) perror("write");
            }
        }
    }
    if (test) printf("\n보냄 %d · 받음 %d · 응답 없음 %d\n", seq, got, lost);
    close(ep); if (tfd >= 0) close(tfd); close(fd);
    printf("종료\n");
    return EXIT_SUCCESS;
}
