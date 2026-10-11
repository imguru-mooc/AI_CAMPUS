# Makefile — Embedded I/O 제어 공통 템플릿 (Day 1~5 같은 파일 사용)
# 사용법: make 이름  (예: make 01_board_info)
#         make       (폴더의 .c 전부)   /   make clean
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
LDLIBS  = $(shell pkg-config --libs libgpiod 2>/dev/null) -lpthread

SRCS = $(wildcard *.c)
BINS = $(SRCS:.c=)

all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(BINS)

.PHONY: all clean
