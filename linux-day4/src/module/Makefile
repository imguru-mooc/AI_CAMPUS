# Kbuild 로 커널 모듈 빌드 — 실행 중인 커널의 헤더가 필요
# sudo apt install -y linux-headers-$(uname -r)
obj-m := mysysfs.o
KDIR  ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
