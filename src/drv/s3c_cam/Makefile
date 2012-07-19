#
# Makefile for the video capture/playback device drivers.
#
KERNEL_DIR ?= /work/prj/teclong/v200/s3c6410/linux-2.6-for_jz

obj-m             += s3c_saa7113h.o
s3c_saa7113h-objs := s3c_camera_driver.o s3c_camif.o s3c_camif_fsm.o saa7113h.o

EXTRA_CFLAGS +=  -I${PWD} -I$(PWD)/inc #-Idrivers/media/dvb/dvb-core -D__DEBUG

all:
	(cd $(KERNEL_DIR); make SUBDIRS=$(PWD) EXTRA_CFLAGS="${EXTRA_CFLAGS}" modules) 

clean:
	-rm -f *.o *.ko *.mod.c .*.o.flags .*.cmd *~ .*.o.d Module* modules*
	-rm -rf .tmp*

