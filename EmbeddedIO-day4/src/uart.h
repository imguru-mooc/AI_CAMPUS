/* uart.h — /dev/serial0 을 termios 로 raw 115200 8N1 설정 + 줄 단위 수신
 *
 *   int fd = uart_open("/dev/serial0", 115200);
 *   linebuf_t lb = {0};
 *   n = read(fd, tmp, sizeof tmp);
 *   for (i...) if (lb_feed(&lb, tmp[i])) printf("%s\n", lb.line);
 *   cs_xor("T=23.0,H=45")   체크섬: '*' 앞 글자들을 XOR
 */
#ifndef UART_H
#define UART_H

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define UART_DEV "/dev/serial0"

static inline speed_t uart_speed(int baud)
{
    switch (baud) {
    case 9600:   return B9600;
    case 19200:  return B19200;
    case 38400:  return B38400;
    case 57600:  return B57600;
    case 115200: return B115200;
    default:     return 0;
    }
}

static inline int uart_open(const char *dev, int baud)
{
    speed_t sp = uart_speed(baud);
    if (!sp) { fprintf(stderr, "지원하지 않는 속도 %d\n", baud); return -1; }
    int fd = open(dev, O_RDWR | O_NOCTTY | O_CLOEXEC);   /* NOCTTY: 제어 터미널로 삼지 않음 */
    if (fd < 0) { perror(dev); return -1; }

    struct termios t;
    if (tcgetattr(fd, &t) < 0) { perror("tcgetattr"); close(fd); return -1; }
    cfmakeraw(&t);                          /* 줄 편집 · 에코 · 특수 문자 처리 모두 끔 */
    t.c_cflag &= ~(CSIZE | PARENB | CSTOPB | CRTSCTS);
    t.c_cflag |= CS8 | CLOCAL | CREAD;      /* 8N1, 모뎀 신호 무시, 수신 켜기 */
    t.c_cc[VMIN]  = 0;                      /* 0 바이트여도 */
    t.c_cc[VTIME] = 1;                      /* 0.1 초 기다리면 read() 가 돌아온다 */
    cfsetispeed(&t, sp);
    cfsetospeed(&t, sp);
    tcflush(fd, TCIOFLUSH);                 /* 열기 전에 쌓인 바이트 버리기 */
    if (tcsetattr(fd, TCSANOW, &t) < 0) { perror("tcsetattr"); close(fd); return -1; }
    return fd;
}

/* 줄 단위 수신 버퍼: '\n' 이 오면 1 을 돌려주고 line 에 한 줄이 들어 있다 */
typedef struct {
    char line[128];
    size_t n;
    int overflow;
} linebuf_t;

static inline int lb_feed(linebuf_t *lb, char c)
{
    if (c == '\r') return 0;
    if (c == '\n') {
        int ok = !lb->overflow;
        lb->line[lb->n] = '\0';
        lb->n = 0; lb->overflow = 0;
        return ok;                          /* 너무 긴 줄은 통째로 버린다 */
    }
    if (lb->n < sizeof lb->line - 1) lb->line[lb->n++] = c;
    else lb->overflow = 1;
    return 0;
}

static inline uint8_t cs_xor(const char *s)
{
    uint8_t x = 0;
    while (*s && *s != '*') x ^= (uint8_t)*s++;
    return x;
}

/* "본문*HH" 형식인지, 체크섬이 맞는지: 1 맞음 · 0 틀림 · -1 '*' 없음 */
static inline int cs_check(const char *line)
{
    const char *star = strchr(line, '*');
    if (!star) return -1;
    unsigned int got;
    if (sscanf(star + 1, "%2X", &got) != 1) return 0;
    return cs_xor(line) == got;
}

/* 본문 뒤에 *HH\n 을 붙여 보낸다 */
static inline int uart_send_line(int fd, const char *body)
{
    char buf[160];
    int n = snprintf(buf, sizeof buf, "%s*%02X\n", body, cs_xor(body));
    return write(fd, buf, n) == n ? 0 : -1;
}

#endif
