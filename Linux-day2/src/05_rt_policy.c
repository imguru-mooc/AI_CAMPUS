/* 05_rt_policy.c — 스케줄링 정책 확인과 SCHED_FIFO 로 바꾸기
 * 빌드: make 05_rt_policy
 * 실행: ./05_rt_policy          (일반 사용자: 정책 변경 실패 EPERM 확인)
 *       sudo ./05_rt_policy 50  (SCHED_FIFO 우선순위 50)
 *       chrt -p <PID>           (다른 PuTTY 창에서 확인)
 */
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

static const char *policy_name(int p)
{
    switch (p) {
    case SCHED_OTHER: return "SCHED_OTHER";
    case SCHED_FIFO:  return "SCHED_FIFO";
    case SCHED_RR:    return "SCHED_RR";
    default:          return "기타";
    }
}

static long read_long(const char *path)
{
    FILE *fp = fopen(path, "r");
    long v = -1;
    if (fp) {
        if (fscanf(fp, "%ld", &v) != 1)
            v = -1;
        fclose(fp);
    }
    return v;
}

int main(int argc, char *argv[])
{
    int prio = (argc > 1) ? atoi(argv[1]) : 50;

    printf("PID=%d  현재 정책=%s\n", getpid(), policy_name(sched_getscheduler(0)));
    printf("FIFO 우선순위 범위: %d ~ %d\n", sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
    printf("RT throttling: %ld us / %ld us 동안만 RT 태스크 실행 허용\n",
           read_long("/proc/sys/kernel/sched_rt_runtime_us"), read_long("/proc/sys/kernel/sched_rt_period_us"));

    struct sched_param sp = { .sched_priority = prio };
    if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1) {
        perror("sched_setscheduler");             /* 일반 사용자 → EPERM */
        return 1;
    }
    struct sched_param got;
    sched_getparam(0, &got);
    printf("변경 후 정책=%s, 우선순위=%d\n", policy_name(sched_getscheduler(0)), got.sched_priority);
    printf("10초 동안 대기합니다. 다른 창에서: chrt -p %d\n", getpid());
    sleep(10);
    return 0;
}
