# Linux & Shell Day 4 — 공통 Makefile (파일시스템 · sysfs)
# make          : 모든 예제 빌드
# make clean    : 실행 파일 삭제
CC      = gcc
CFLAGS  = -Wall -Wextra -g
SRCS    = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.bin out*.dat

.PHONY: all clean
