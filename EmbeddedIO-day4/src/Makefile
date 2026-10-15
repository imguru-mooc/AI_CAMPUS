# Makefile — Embedded I/O 제어 공통 템플릿 (Day 1~5 같은 파일 사용)
# 사용법: make 이름  (예: make 01_swpwm)
#         make       (폴더의 .c 전부)   /   make clean
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g $(shell pkg-config --cflags libgpiod 2>/dev/null)
LDLIBS  = $(shell pkg-config --libs libgpiod 2>/dev/null) -lpthread -lm

SRCS = $(wildcard *.c)
BINS = $(SRCS:.c=)

all: $(BINS)

# 같은 폴더의 공통 헤더(gpio_common.h)가 바뀌어도 다시 빌드
%: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(BINS)

.PHONY: all clean
