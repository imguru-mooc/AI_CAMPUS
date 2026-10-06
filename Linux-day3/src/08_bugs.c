/* 08_bugs.c — 메모리 오류 4가지를 일부러 만들고 도구로 잡기
 * 빌드: make 08_bugs        (일반)
 *       make asan           (AddressSanitizer → 08_bugs_asan)
 * 실행: valgrind --leak-check=full ./08_bugs leak
 *       ./08_bugs_asan uaf | overflow | double
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void leak(void)
{
    char *p = malloc(100);
    strcpy(p, "free 를 잊었다");
    printf("%s\n", p);
}                                       /* p 를 잃어버림 → 100 바이트 누수 */

static void use_after_free(void)
{
    int *p = malloc(4 * sizeof(int));
    p[0] = 10;
    free(p);
    printf("해제 후 읽기: %d\n", p[0]);   /* 이미 돌려준 메모리 */
}

static void overflow(void)
{
    char *p = malloc(8);
    strcpy(p, "123456789");             /* 10 바이트(널 포함)를 8 바이트에 */
    printf("%s\n", p);
    free(p);
}

static void double_free(void)
{
    char *p = malloc(16);
    free(p);
    free(p);                            /* 두 번 해제 */
}

int main(int argc, char *argv[])
{
    const char *k = (argc > 1) ? argv[1] : "leak";
    if (strcmp(k, "leak") == 0) leak();
    else if (strcmp(k, "uaf") == 0) use_after_free();
    else if (strcmp(k, "overflow") == 0) overflow();
    else if (strcmp(k, "double") == 0) double_free();
    else fprintf(stderr, "사용법: %s leak|uaf|overflow|double\n", argv[0]);
    return 0;
}
