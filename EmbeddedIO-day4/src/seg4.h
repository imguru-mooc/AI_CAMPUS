/* seg4.h — 4자리 7-Segment 멀티플렉싱 (595 1개 + 자리 선택 GPIO 4개)
 *
 *   세그먼트 a~dp : 595 Q0~Q7 → 1kΩ → 4자리 모듈의 a~dp (네 자리가 공유)
 *   자리 D1~D4    : GPIO17 · 27 · 22 · 5  (공통 음극: LOW = 그 자리 켜짐)
 *
 *   seg4_t d;
 *   seg4_start(&d, spi_fd, fifo);      표시 쓰레드 시작 (자리당 2 ms → 약 125 Hz)
 *   seg4_text(&d, "23.4");             오른쪽 정렬, '.' 은 앞 글자의 dp
 *   seg4_stop(&d);                     쓰레드 종료 · 끄기 · 반납
 */
#ifndef SEG4_H
#define SEG4_H

#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include "gpio_common.h"
#include "spi595.h"

#define SEG4_DWELL_NS 2000000L

static const unsigned int SEG4_DIG[4] = { 17, 27, 22, 5 };   /* D1(왼쪽) ~ D4(오른쪽) */

typedef struct {
    int spi;
    struct gpiod_line_request *dig;
    uint8_t buf[4];                 /* 화면 버퍼 — 쓰레드와 함께 쓰므로 mx 로 보호 */
    pthread_mutex_t mx;
    pthread_t th;
    volatile int run;
    long max_late_us;               /* 확인용: 가장 늦게 깨어난 시간 */
} seg4_t;

static void *seg4_thread(void *arg)
{
    seg4_t *d = arg;
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    enum gpiod_line_value v[4];
    for (int k = 0; d->run; k = (k + 1) % 4) {
        pthread_mutex_lock(&d->mx);
        uint8_t pat = d->buf[k];
        pthread_mutex_unlock(&d->mx);

        for (int i = 0; i < 4; i++) v[i] = GPIOD_LINE_VALUE_INACTIVE;
        gpiod_line_request_set_values(d->dig, v);          /* ① 모든 자리 끄기 */
        spi_send(d->spi, pat);                              /* ② 모양 바꾸기 */
        v[k] = GPIOD_LINE_VALUE_ACTIVE;
        gpiod_line_request_set_values(d->dig, v);          /* ③ 이 자리만 켜기 */

        next.tv_nsec += SEG4_DWELL_NS;
        if (next.tv_nsec >= 1000000000L) { next.tv_nsec -= 1000000000L; next.tv_sec++; }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        struct timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
        long late = ((now.tv_sec - next.tv_sec) * 1000000000L + now.tv_nsec - next.tv_nsec) / 1000;
        if (late > d->max_late_us) d->max_late_us = late;
    }
    for (int i = 0; i < 4; i++) v[i] = GPIOD_LINE_VALUE_INACTIVE;
    gpiod_line_request_set_values(d->dig, v);
    return NULL;
}

static inline int seg4_start(seg4_t *d, int spi_fd, int fifo)
{
    memset(d, 0, sizeof *d);
    d->spi = spi_fd;
    pthread_mutex_init(&d->mx, NULL);

    /* 자리 선택은 active-low 로 요청: 코드의 1(ACTIVE) = 핀 LOW = 자리 켜짐 (Day 2 2교시) */
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_active_low(s, true);
    gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);   /* 처음엔 모두 꺼짐 */
    d->dig = gpio_request("seg4", SEG4_DIG, 4, s);
    gpiod_line_settings_free(s);
    if (!d->dig) return -1;

    pthread_attr_t at;
    pthread_attr_init(&at);
    if (fifo) {
        if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) perror("mlockall (계속 진행)");
        struct sched_param sp = { .sched_priority = 50 };
        pthread_attr_setinheritsched(&at, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&at, SCHED_FIFO);
        pthread_attr_setschedparam(&at, &sp);
    }
    d->run = 1;
    int e = pthread_create(&d->th, &at, seg4_thread, d);
    if (e && fifo) {                                          /* 권한이 없으면 일반 쓰레드로 */
        fprintf(stderr, "SCHED_FIFO 실패(%s) — setcap 필요. 일반 쓰레드로 계속\n", strerror(e));
        e = pthread_create(&d->th, NULL, seg4_thread, d);
    }
    pthread_attr_destroy(&at);
    if (e) { fprintf(stderr, "pthread_create: %s\n", strerror(e)); return -1; }
    return 0;
}

/* 패턴 4개를 그대로 */
static inline void seg4_raw(seg4_t *d, const uint8_t p[4])
{
    pthread_mutex_lock(&d->mx);
    memcpy(d->buf, p, 4);
    pthread_mutex_unlock(&d->mx);
}

/* 글자열 → 오른쪽 정렬. "23.4" → [ ][2][3.][4] */
static inline void seg4_text(seg4_t *d, const char *s)
{
    uint8_t p[4] = { 0, 0, 0, 0 };
    int k = 3;
    for (int i = (int)strlen(s) - 1; i >= 0 && k >= 0; i--) {
        if (s[i] == '.') {
            if (i > 0) { p[k] = seg_char(s[i - 1]) | SEG_DP; i--; }
            else p[k] = SEG_DP;
        } else p[k] = seg_char(s[i]);
        k--;
    }
    seg4_raw(d, p);
}

static inline void seg4_stop(seg4_t *d)
{
    if (!d->run) return;
    d->run = 0;
    pthread_join(d->th, NULL);
    spi_send(d->spi, 0x00);
    gpiod_line_request_release(d->dig);
    pthread_mutex_destroy(&d->mx);
}

#endif
