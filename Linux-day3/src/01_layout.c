/* 01_layout.c — 변수 종류별 주소와 /proc/self/maps 대조
 * 빌드: make 01_layout
 * 실행: ./01_layout             (두 번 실행해 주소가 바뀌는지 — ASLR)
 *       setarch -R ./01_layout  (ASLR 끄고 실행 — 매번 같은 주소)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int g_init = 42;              /* data : 초기값 있는 전역 */
int g_zero;                   /* bss  : 초기값 없는 전역 */
const char *g_msg = "hello";  /* "hello" 는 rodata */

static void show_maps(void)
{
    FILE *fp = fopen("/proc/self/maps", "r");
    char line[512];
    if (fp == NULL)
        return;
    printf("\n--- /proc/self/maps (주요 줄) ---\n");
    while (fgets(line, sizeof(line), fp) != NULL)
        if (strstr(line, "01_layout") || strstr(line, "[heap]") || strstr(line, "[stack]"))
            printf("%s", line);
    fclose(fp);
}

int main(void)
{
    static int s_local = 7;   /* data : 함수 안 static */
    int local = 1;            /* stack */
    int *small = malloc(64);           /* heap (brk 영역) */
    char *big = malloc(1024 * 1024);   /* 1MB → mmap 영역 */

    printf("%-26s %p\n", "text   main()", (void *)main);
    printf("%-26s %p\n", "rodata \"hello\"", (void *)g_msg);
    printf("%-26s %p\n", "data   g_init", (void *)&g_init);
    printf("%-26s %p\n", "data   static s_local", (void *)&s_local);
    printf("%-26s %p\n", "bss    g_zero", (void *)&g_zero);
    printf("%-26s %p\n", "heap   malloc(64)", (void *)small);
    printf("%-26s %p\n", "mmap   malloc(1MB)", (void *)big);
    printf("%-26s %p\n", "stack  local", (void *)&local);
    show_maps();

    free(small);
    free(big);
    return 0;
}
