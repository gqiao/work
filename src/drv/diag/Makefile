
ifneq ($(KERNELRELEASE),)
obj-m += diag.o 
diag-objs := diag_main.o mem.o irq.o dma.o

else
KDIR ?= /lib/modules/$(shell uname -r)/build

modules:
	$(MAKE) -C $(KDIR) SUBDIRS=$(shell pwd) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(shell pwd) clean
	rm -rf *~ 
endif
