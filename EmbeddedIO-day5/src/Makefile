# Makefile — Day 5 (모듈 여러 개를 링크)
#   make            예제 · envmon · eio.dtbo 모두
#   make envmon     종합 프로젝트만
#   make clean
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g $(shell pkg-config --cflags libgpiod 2>/dev/null)
LDLIBS  = $(shell pkg-config --libs libgpiod 2>/dev/null) -lpthread -lm

BINS = 01_dht_poll 02_dht_iio 03_led_key 04_lcd_demo envmon

all: $(BINS) eio.dtbo

01_dht_poll: 01_dht_poll.c gpio_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)
02_dht_iio: 02_dht_iio.c dht.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)
03_led_key: 03_led_key.c gpio_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)
04_lcd_demo: 04_lcd_demo.c lcd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# 종합 프로젝트: 모듈 .o 를 따로 컴파일해 한 번에 링크
envmon: envmon.o lcd.o dht.o sonar.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c *.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Device Tree overlay
eio.dtbo: eio-overlay.dts
	dtc -@ -I dts -O dtb -o $@ $<

clean:
	rm -f $(BINS) *.o eio.dtbo

.PHONY: all clean
