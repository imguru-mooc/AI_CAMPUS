/* 06_sysread.c — procfs · sysfs 값을 C 로 읽기
 * 빌드: make 06_sysread
 * 실행: ./06_sysread
 * sysfs 규칙: 파일 하나 = 값 하나 (텍스트 한 줄)
 */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

/* 파일의 첫 줄을 읽어 buf 에. 성공 0, 실패 -1 */
static int read_line(const char *path, char *buf, size_t size)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return -1;
    if (fgets(buf, size, fp) == NULL) {
        fclose(fp);
        return -1;
    }
    buf[strcspn(buf, "\n")] = '\0';
    fclose(fp);
    return 0;
}

int main(void)
{
    char v[256], path[512];

    if (read_line("/sys/devices/system/cpu/online", v, sizeof(v)) == 0)
        printf("온라인 CPU            : %s\n", v);
    if (read_line("/proc/loadavg", v, sizeof(v)) == 0)
        printf("loadavg (procfs)      : %s\n", v);

    DIR *d = opendir("/sys/class/net");          /* 네트워크 장치마다 디렉터리 하나 */
    struct dirent *e;
    while (d && (e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.')
            continue;
        char rx[64] = "?", tx[64] = "?", st[64] = "?";
        snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_bytes", e->d_name);
        read_line(path, rx, sizeof(rx));
        snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_bytes", e->d_name);
        read_line(path, tx, sizeof(tx));
        snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", e->d_name);
        read_line(path, st, sizeof(st));
        printf("net %-8s %-7s rx %12s B  tx %12s B\n", e->d_name, st, rx, tx);
    }
    if (d)
        closedir(d);

    const char *disks[] = { "sda", "vda", "nvme0n1" };
    for (int i = 0; i < 3; i++) {
        snprintf(path, sizeof(path), "/sys/block/%s/stat", disks[i]);
        if (read_line(path, v, sizeof(v)) == 0) {
            unsigned long rd, rm, rs, rt, wr;
            sscanf(v, "%lu %lu %lu %lu %lu", &rd, &rm, &rs, &rt, &wr);
            printf("disk %-8s 읽기 %lu 회, 쓰기 %lu 회 (/sys/block/%s/stat)\n", disks[i], rd, wr, disks[i]);
        }
    }

    if (access("/sys/class/thermal/thermal_zone0/temp", R_OK) == 0 &&
        read_line("/sys/class/thermal/thermal_zone0/temp", v, sizeof(v)) == 0)
        printf("온도                  : %s (1/1000 ℃)\n", v);
    else
        printf("온도                  : thermal_zone 없음 (VirtualBox VM 에는 온도 센서가 없다)\n");
    return 0;
}
