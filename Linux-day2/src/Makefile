# Linux & Shell Day 2 — 공통 Makefile (스케줄링)
# make        : 모든 예제 빌드
# make clean  : 실행 파일 삭제
CC      = gcc
CFLAGS  = -Wall -Wextra -g
SRCS    = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
