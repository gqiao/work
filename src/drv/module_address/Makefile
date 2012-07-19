KERNEL_DIR ?= /work/prj/ubuntu-maverick


obj-m += module_address.o
module_address-objs := __module_address.o

EXTRA_CFLAGS += -D__DEBUG

all:
	(cd $(KERNEL_DIR); make SUBDIRS=$(PWD) EXTRA_CFLAGS="${EXTRA_CFLAGS}" modules) 
	-rm -rf *.o *.mod.c .*.o.flags .*.cmd *~ .tmp* modules.order Module.symvers

clean:
	-rm -rf *.o *.mod.c .*.o.flags .*.cmd *~ .tmp* modules.order Module.symvers *.ko 

