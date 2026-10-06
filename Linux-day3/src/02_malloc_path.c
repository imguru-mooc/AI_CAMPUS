/* 02_malloc_path.c — malloc 이 brk(heap) 와 mmap 중 무엇을 쓰는지 확인
 * 빌드: make 02_malloc_path
 * 실행: ./02_malloc_path 64        (64KB)
 *       ./02_malloc_path 256       (256KB)
 *       strace -e trace=brk,mmap,munmap ./02_malloc_path 256
 *       ./02_malloc_path 256 512   (mallopt 로 기준을 512KB 로 올리면?)
 */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    size_t kb = (argc > 1) ? strtoul(argv[1], NULL, 10) : 64;
    if (argc > 2)
        mallopt(M_MMAP_THRESHOLD, (int)strtoul(argv[2], NULL, 10) * 1024);

    char *heap_start = sbrk(0);                /* 프로그램 시작 시 heap 위치 */
    free(malloc(1));                           /* malloc 초기화(첫 132KB 준비)를 미리 끝낸다 */
    void *brk_before = sbrk(0);                /* 현재 heap 의 끝 */
    char *p = malloc(kb * 1024);
    void *brk_after = sbrk(0);
    if (p == NULL) {
        perror("malloc");
        return 1;
    }
    int in_heap = (p >= heap_start && p < (char *)brk_after);
    printf("malloc(%zu KB) = %p\n", kb, (void *)p);
    printf("heap 끝(brk): %p → %p  (%+ld KB)\n", brk_before, brk_after,
           ((char *)brk_after - (char *)brk_before) / 1024);
    printf("판정: %s\n", in_heap ? "heap (brk 영역)" : "mmap (독립된 영역 — free 하면 munmap 으로 바로 반환)");
    free(p);
    printf("free 후 brk: %p\n", sbrk(0));
    return 0;
}
