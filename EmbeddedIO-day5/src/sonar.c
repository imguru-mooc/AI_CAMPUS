/* sonar.c — sonar.h 구현: Echo 의 rising · falling 커널 타임스탬프 차이 */
#include <poll.h>
#include <stdlib.h>
#include "gpio_common.h"
#include "sonar.h"

struct sonar {
    unsigned int trig, echo;
    struct gpiod_line_request *t, *e;
    struct gpiod_edge_event_buffer *buf;
};

sonar_t *sonar_open(unsigned int trig, unsigned int echo)
{
    sonar_t *s = calloc(1, sizeof *s);
    if (!s) return NULL;
    s->trig = trig; s->echo = echo;
    s->t = gpio_request_output("sonar", &s->trig, 1);
    struct gpiod_line_settings *st = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(st, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(st, GPIOD_LINE_EDGE_BOTH);
    gpiod_line_settings_set_bias(st, GPIOD_LINE_BIAS_DISABLED);
    s->e = gpio_request("sonar", &s->echo, 1, st);
    gpiod_line_settings_free(st);
    s->buf = gpiod_edge_event_buffer_new(8);
    if (!s->t || !s->e || !s->buf) { sonar_close(s); return NULL; }
    return s;
}

double sonar_measure(sonar_t *s)
{
    struct pollfd p = { .fd = gpiod_line_request_get_fd(s->e), .events = POLLIN };
    while (poll(&p, 1, 0) > 0) gpiod_line_request_read_edge_events(s->e, s->buf, 8);   /* 찌꺼기 버림 */

    gpiod_line_request_set_value(s->t, s->trig, GPIOD_LINE_VALUE_ACTIVE);
    struct timespec ts = { 0, 10000 };
    nanosleep(&ts, NULL);
    gpiod_line_request_set_value(s->t, s->trig, GPIOD_LINE_VALUE_INACTIVE);

    long long rise = 0, fall = 0, deadline = now_ns() + 60000000LL;
    while (!fall) {
        int left = (int)((deadline - now_ns()) / 1000000);
        if (left <= 0 || poll(&p, 1, left) <= 0) return -1;
        int n = gpiod_line_request_read_edge_events(s->e, s->buf, 8);
        for (int i = 0; i < n; i++) {
            struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(s->buf, i);
            long long t = (long long)gpiod_edge_event_get_timestamp_ns(ev);
            if (gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_RISING_EDGE) rise = t;
            else if (rise) { fall = t; break; }
        }
    }
    double us = (fall - rise) / 1000.0;
    return us > 30000 ? -1 : us * 0.0343 / 2;
}

static int cmp(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

double sonar_median(sonar_t *s, int n)
{
    double v[7]; int k = 0;
    if (n > 7) n = 7;
    for (int i = 0; i < n; i++) {
        double d = sonar_measure(s);
        if (d > 0) v[k++] = d;
        sleep_ms(60);
    }
    if (!k) return -1;
    qsort(v, (size_t)k, sizeof v[0], cmp);
    return v[k / 2];
}

void sonar_close(sonar_t *s)
{
    if (!s) return;
    if (s->t) { gpiod_line_request_set_value(s->t, s->trig, GPIOD_LINE_VALUE_INACTIVE); gpiod_line_request_release(s->t); }
    if (s->e) gpiod_line_request_release(s->e);
    if (s->buf) gpiod_edge_event_buffer_free(s->buf);
    free(s);
}
