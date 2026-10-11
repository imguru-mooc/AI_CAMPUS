# Linux & Shell Day 5 — 공통 Makefile (쓰레드 · 동기화)
# make        : 모든 예제 빌드 (-pthread)
# make clean  : 실행 파일 삭제
CC      = gcc
CFLAGS  = -Wall -Wextra -g -O1 -pthread
LDLIBS  = -lm
SRCS    = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGETS) *.csv

.PHONY: all clean
