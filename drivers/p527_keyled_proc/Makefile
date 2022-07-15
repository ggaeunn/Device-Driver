APP := keyled_proc_app
MOD := keyled_proc_dev
SRC := $(APP).c
obj-m := $(MOD).o
PWD := $(shell pwd)

#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /home/ubuntu/sm7/kernel/linux
CC := arm-linux-gnueabi-gcc

default:$(APP)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
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
