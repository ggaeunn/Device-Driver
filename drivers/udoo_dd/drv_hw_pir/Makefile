APP := drv_hw_pir_test
MOD := drv_hw_pir
SRC := $(APP).c
obj-m := $(MOD).o
PWD := $(shell pwd)
ARCH = ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

#KDIR := /lib/modules/$(shell uname -r)/build
#CC := gcc
KDIR := /home/ubuntu/udoo_linux_bsp/kernel/linux_kernel-3.14-1.0.x-udoo
CC := arm-linux-gnueabihf-gcc

APPLDFLAGS := -D_REENTRANT -pthread


default:$(APP)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules $(ARCH)
	cp $(MOD).ko /srv/nfs

$(APP):$(SRC)
	$(CC) -o $@ $< $(APPLDFLAGS)
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
