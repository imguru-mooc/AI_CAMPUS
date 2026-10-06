# Linux & Shell Day 3 — 공통 Makefile (메모리)
# make          : 모든 예제 빌드
# make asan     : 08_bugs 를 AddressSanitizer 로 빌드 (08_bugs_asan)
# make clean    : 실행 파일 삭제
CC      = gcc
CFLAGS  = -Wall -Wextra -g
SRCS    = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

asan: 08_bugs.c
	$(CC) $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer -o 08_bugs_asan 08_bugs.c

clean:
	rm -f $(TARGETS) 08_bugs_asan

.PHONY: all asan clean
