KERNEL_DIR ?= /work/prj/teclong/v200/s3c6410/linux-2.6

obj-m   += s3c_tvscaler.o
s3c_tvscaler-objs += s3c-tvscaler.o

obj-m	+= s3c_tvenc.o
s3c_tvenc-objs += s3c-tvenc.o

EXTRA_CFLAGS += -D__DEBUG -I$(PWD)/../../inc

release:
	(cd $(KERNEL_DIR); make SUBDIRS=$(PWD) EXTRA_CFLAGS="${EXTRA_CFLAGS}" modules) 

clean:
	-rm -f *.o *.ko *.mod.c .*.o.flags .*.cmd *~
	-rm -rf .tmp*

debug: 
	(cd $(KERNEL_DIR); make SUBDIRS=$(PWD) EXTRA_CFLAGS="${EXTRA_CFLAGS}" modules) 

