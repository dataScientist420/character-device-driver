# -*- Makefile -*-

SRC := kernel_src
obj-m := $(SRC)/chardriver.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	-rm -f $(SRC)/*.o $(SRC)/*.ko $(SRC)/.*.cmd $(SRC)/.*.flags $(SRC)/*.mod.c Module.symvers modules.order
