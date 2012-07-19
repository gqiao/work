KERNEL_DIR ?= /work/prj/teclong/v200/s3c6410/linux-2.6


obj-m += s3c_mfc10.o
s3c_mfc10-objs += prism_s_v137.o s3c_mfc_bitproc_buf.o s3c_mfc.o s3c_mfc_databuf.o s3c_mfc_init_hw.o s3c_mfc_instance.o s3c_mfc_inst_pool.o s3c_mfc_set_config.o s3c_mfc_sfr.o s3c_mfc_yuv_buf_manager.o

EXTRA_CFLAGS += -DLINUX
EXTRA_CFLAGS += -DDIVX_ENABLE
EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -I$(PWD)/../../inc #-D__DEBUG 


all:
	(cd $(KERNEL_DIR); make SUBDIRS=$(PWD) EXTRA_CFLAGS="${EXTRA_CFLAGS}" modules) 

clean:
	(for i in `find . -type d`; do (cd $$i && rm -rf *.o *.ko *.mod.c .*.o.flags .*.cmd *~); done)
	-rm -rf .tmp*

