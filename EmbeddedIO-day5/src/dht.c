/* dht.c — dht.h 구현 */
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "dht.h"

static int read_long(const char *dir, const char *name, long *v)
{
    char path[160];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    int ok = fscanf(f, "%ld", v) == 1;      /* 드라이버가 실패하면 read 가 EIO · ETIMEDOUT */
    fclose(f);
    return ok ? 0 : -1;
}

int dht_open(dht_t *d)
{
    glob_t g;
    memset(d, 0, sizeof *d);
    /* 장치 번호(N)는 부팅마다 바뀔 수 있으므로 습도 파일이 있는 장치를 찾는다 */
    if (glob("/sys/bus/iio/devices/iio:device*/in_humidityrelative_input", 0, NULL, &g) != 0) return -1;
    snprintf(d->dir, sizeof d->dir, "%.*s", (int)(strrchr(g.gl_pathv[0], '/') - g.gl_pathv[0]), g.gl_pathv[0]);
    globfree(&g);
    return 0;
}

int dht_read(dht_t *d, double *temp_c, double *humi)
{
    for (int i = 0; i < 3; i++) {
        long t, h;
        if (read_long(d->dir, "in_temp_input", &t) == 0 &&
            read_long(d->dir, "in_humidityrelative_input", &h) == 0) {
            *temp_c = t / 1000.0;
            *humi = h / 1000.0;
            d->fails = 0;
            return 0;
        }
        struct timespec ts = { 0, 300000000L };    /* 0.3 초 뒤 다시 */
        nanosleep(&ts, NULL);
    }
    d->fails++;
    return -1;
}
