/* pwm_sysfs.h — 하드웨어 PWM 을 /sys/class/pwm 으로 다루는 도우미 (Day 3~5 공통)
 *
 *   pwm_open(&p, 0, ch)            pwmchip0 의 채널 ch 를 export (GPIO18 = 0, GPIO19 = 1)
 *   pwm_set(&p, period_ns, duty_ns) 주기 · 듀티를 순서에 맞게 기록
 *   pwm_enable(&p, 1/0)            출력 켜기/끄기
 *   pwm_close(&p)                  끄고 unexport
 *
 * 전제: config.txt 에 dtoverlay=pwm-2chan (Day 1 5교시), 사용자는 gpio 그룹.
 * 모든 sysfs 값은 "나노초 정수 문자열" 이다.
 */
#ifndef PWM_SYSFS_H
#define PWM_SYSFS_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    char dir[64];          /* /sys/class/pwm/pwmchip0/pwm0 */
    char chip[48];         /* /sys/class/pwm/pwmchip0      */
    int  ch;
    long period;           /* 마지막으로 쓴 값 (ns) */
    long duty;
} pwm_t;

static inline int pwm_write(const char *dir, const char *name, long v)
{
    char path[96], buf[24];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    int n = snprintf(buf, sizeof buf, "%ld", v);
    ssize_t w = write(fd, buf, (size_t)n);
    int saved = errno;
    close(fd);
    errno = saved;
    return w < 0 ? -1 : 0;
}

static inline int pwm_open(pwm_t *p, int chip, int ch)
{
    memset(p, 0, sizeof *p);
    p->ch = ch;
    snprintf(p->chip, sizeof p->chip, "/sys/class/pwm/pwmchip%d", chip);
    snprintf(p->dir, sizeof p->dir, "%s/pwm%d", p->chip, ch);

    if (access(p->dir, F_OK) != 0 && pwm_write(p->chip, "export", ch) < 0 && errno != EBUSY) {
        fprintf(stderr, "pwm export 실패 (%s): %s — config.txt 의 dtoverlay=pwm-2chan 확인\n",
                p->chip, strerror(errno));
        return -1;
    }
    /* export 직후에는 udev 가 권한(gpio 그룹)을 고칠 때까지 잠깐 기다려야 한다 */
    char per[96];
    snprintf(per, sizeof per, "%s/period", p->dir);
    for (int i = 0; i < 100 && access(per, W_OK) != 0; i++) {
        struct timespec ts = { 0, 10 * 1000000L };
        nanosleep(&ts, NULL);
    }
    if (access(per, W_OK) != 0) {
        fprintf(stderr, "%s 에 쓸 권한이 없음 — gpio 그룹 확인\n", per);
        return -1;
    }
    return 0;
}

/* duty ≤ period 를 항상 지키도록 쓰는 순서를 고른다 */
static inline int pwm_set(pwm_t *p, long period_ns, long duty_ns)
{
    if (duty_ns > period_ns) duty_ns = period_ns;
    if (duty_ns < 0) duty_ns = 0;
    int r;
    if (period_ns >= p->period) {          /* 주기를 늘릴 때: period 먼저 */
        r = pwm_write(p->dir, "period", period_ns);
        if (r == 0) r = pwm_write(p->dir, "duty_cycle", duty_ns);
    } else {                               /* 주기를 줄일 때: duty 먼저 */
        r = pwm_write(p->dir, "duty_cycle", duty_ns);
        if (r == 0) r = pwm_write(p->dir, "period", period_ns);
    }
    if (r < 0) { perror("pwm_set"); return -1; }
    p->period = period_ns;
    p->duty = duty_ns;
    return 0;
}

static inline int pwm_set_duty(pwm_t *p, long duty_ns) { return pwm_set(p, p->period, duty_ns); }

static inline int pwm_enable(pwm_t *p, int on) { return pwm_write(p->dir, "enable", on ? 1 : 0); }

static inline void pwm_close(pwm_t *p)
{
    pwm_enable(p, 0);
    pwm_write(p->chip, "unexport", p->ch);
}

#endif
