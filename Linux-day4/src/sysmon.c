/* sysmon.c — procfs · sysfs 로 만든 시스템 모니터 (Day 4 종합)
 * 빌드: make sysmon
 * 실행: ./sysmon -i 1 -n 10 -o sys.csv          (1초 주기, 10번)
 *       ./sysmon -d sda -N enp0s3              (디스크 · 네트워크 장치 지정)
 *       Ctrl+C 로 중간에 끝내도 CSV 는 저장된다
 * 출력: time,cpu_pct,mem_used_mb,disk_write_kbps,net_rx_kbps
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

static volatile sig_atomic_t g_stop = 0;
static void on_sigint(int sig) { (void)sig; g_stop = 1; }

static unsigned long long read_ull(const char *path, int field)   /* field 번째 숫자 (1부터) */
{
    FILE *fp = fopen(path, "r");
    unsigned long long v = 0, x;
    if (fp == NULL)
        return 0;
    for (int i = 1; i <= field && fscanf(fp, "%llu", &x) == 1; i++)
        if (i == field)
            v = x;
    fclose(fp);
    return v;
}

static void cpu_times(unsigned long long *total, unsigned long long *idle)
{
    unsigned long long u = 0, n = 0, s = 0, id = 0, io = 0, irq = 0, sirq = 0;
    FILE *fp = fopen("/proc/stat", "r");
    if (fp) {
        if (fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu", &u, &n, &s, &id, &io, &irq, &sirq) != 7)
            u = n = s = id = io = irq = sirq = 0;
        fclose(fp);
    }
    *total = u + n + s + id + io + irq + sirq;
    *idle = id + io;
}

static long mem_used_mb(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    char line[128];
    long total = 0, avail = 0;
    while (fp && fgets(line, sizeof(line), fp)) {
        sscanf(line, "MemTotal: %ld", &total);
        sscanf(line, "MemAvailable: %ld", &avail);
    }
    if (fp)
        fclose(fp);
    return (total - avail) / 1024;
}

static void first_net(char *out, size_t size)     /* lo 가 아닌 첫 네트워크 장치 */
{
    DIR *d = opendir("/sys/class/net");
    struct dirent *e;
    snprintf(out, size, "lo");
    while (d && (e = readdir(d)) != NULL)
        if (e->d_name[0] != '.' && strcmp(e->d_name, "lo") != 0) {
            snprintf(out, size, "%s", e->d_name);
            break;
        }
    if (d)
        closedir(d);
}

int main(int argc, char *argv[])
{
    int interval = 1, count = 10, opt;
    const char *outpath = "sys.csv";
    char disk[32] = "", net[64] = "";
    while ((opt = getopt(argc, argv, "i:n:o:d:N:")) != -1) {
        switch (opt) {
        case 'i': interval = atoi(optarg); break;
        case 'n': count = atoi(optarg); break;
        case 'o': outpath = optarg; break;
        case 'd': snprintf(disk, sizeof(disk), "%s", optarg); break;
        case 'N': snprintf(net, sizeof(net), "%s", optarg); break;
        default:
            fprintf(stderr, "사용법: %s [-i 초] [-n 횟수] [-o csv] [-d 디스크] [-N 네트워크장치]\n", argv[0]);
            return 1;
        }
    }
    if (!disk[0]) {                                    /* sda · vda · nvme0n1 중 있는 것 */
        const char *c[] = { "sda", "vda", "nvme0n1" };
        for (int i = 0; i < 3 && !disk[0]; i++) {
            char p[64];
            snprintf(p, sizeof(p), "/sys/block/%s/stat", c[i]);
            if (access(p, R_OK) == 0)
                snprintf(disk, sizeof(disk), "%s", c[i]);
        }
    }
    if (!net[0])
        first_net(net, sizeof(net));
    char dpath[128], npath[160];
    snprintf(dpath, sizeof(dpath), "/sys/block/%s/stat", disk);
    snprintf(npath, sizeof(npath), "/sys/class/net/%s/statistics/rx_bytes", net);

    FILE *out = fopen(outpath, "w");
    if (out == NULL) {
        perror(outpath);
        return 1;
    }
    signal(SIGINT, on_sigint);
    fprintf(out, "time,cpu_pct,mem_used_mb,disk_write_kbps,net_rx_kbps\n");
    printf("디스크=%s 네트워크=%s 주기=%d초 → %s\n", disk, net, interval, outpath);
    printf("%4s %7s %8s %12s %10s\n", "초", "CPU%", "메모리MB", "디스크쓰기KB/s", "수신KB/s");

    unsigned long long pt, pi, ps = read_ull(dpath, 7), pr = read_ull(npath, 1);
    cpu_times(&pt, &pi);
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    for (int k = 1; k <= count && !g_stop; k++) {
        next.tv_sec += interval;                       /* Day 2: 절대 시각 주기 */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        if (g_stop)
            break;
        unsigned long long t, i, s = read_ull(dpath, 7), r = read_ull(npath, 1);
        cpu_times(&t, &i);
        double cpu = (t > pt) ? 100.0 * ((t - pt) - (i - pi)) / (t - pt) : 0;
        double dk = (s - ps) * 512.0 / 1024 / interval;
        double nk = (r - pr) / 1024.0 / interval;
        long mem = mem_used_mb();
        fprintf(out, "%d,%.1f,%ld,%.1f,%.1f\n", k * interval, cpu, mem, dk, nk);
        fflush(out);
        printf("%4d %7.1f %8ld %12.1f %10.1f\n", k * interval, cpu, mem, dk, nk);
        pt = t; pi = i; ps = s; pr = r;
    }
    fclose(out);
    printf("저장: %s\n", outpath);
    return 0;
}
