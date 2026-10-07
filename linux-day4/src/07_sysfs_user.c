/* 07_sysfs_user.c — mysysfs.ko 가 만든 /sys/kernel/mysysfs 속성을 읽고 쓰기
 * 빌드: make 07_sysfs_user
 * 실행: sudo insmod module/mysysfs.ko
 *       ./07_sysfs_user            (읽기)
 *       sudo ./07_sysfs_user 42    (쓰기 → 다시 읽기)
 */
#include <stdio.h>
#include <stdlib.h>

#define BASE "/sys/kernel/mysysfs/"

static int read_int(const char *name)
{
    FILE *fp = fopen(BASE "value", "r");
    int v = -1;
    (void)name;
    if (fp == NULL) {
        perror(BASE "value (모듈을 insmod 했나요?)");
        exit(1);
    }
    if (fscanf(fp, "%d", &v) != 1)
        v = -1;
    fclose(fp);
    return v;
}

int main(int argc, char *argv[])
{
    printf("value = %d\n", read_int("value"));
    if (argc > 1) {
        FILE *fp = fopen(BASE "value", "w");
        if (fp == NULL) {
            perror("쓰기 (sudo 필요)");
            return 1;
        }
        fprintf(fp, "%s\n", argv[1]);       /* 커널의 store 콜백이 받는다 */
        if (fclose(fp) != 0) {
            perror("store 거부 (숫자가 아님?)");
            return 1;
        }
        printf("value ← %s, 다시 읽기: %d\n", argv[1], read_int("value"));
        FILE *cf = fopen(BASE "writes", "r");
        int w;
        if (cf && fscanf(cf, "%d", &w) == 1)
            printf("지금까지 쓰기 횟수 writes = %d\n", w);
        if (cf)
            fclose(cf);
    }
    return 0;
}
