APP := call_app_led
MOD := call_dev_led
SRC := $(APP).c
obj-m := $(MOD).o
PWD := $(shell pwd)
ARCH = ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

#KDIR := /lib/modules/$(shell uname -r)/build
#CC := gcc
KDIR := /home/ubuntu/udoo_linux_bsp/kernel/3.14.1
CC := arm-linux-gnueabihf-gcc


default:$(APP)
	make -C $(KDIR) SUBDIRS=$(PWD) modules $(ARCH)
	cp $(MOD).ko /srv/nfs

$(APP):$(SRC)
	$(CC) -o $@ $<
	cp $(APP) /srv/nfs

clean:
	rm -rf *.ko
	rm -rf *.o
	rm -rf .*.cmd
	rm -rf .tmp_*
	rm -rf *.mod.*
	rm -rf Module.*
	rm -rf modules.*
	rm -rf $(APP)
