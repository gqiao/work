KERNEL_DIR ?= /work/prj/teclong/v200/s3c6410/linux-2.6

obj-m            += s3c_tlv320aic23.o
s3c_tlv320aic23-objs += s3c64xx/smdk6410_tlv320aic23.o 
#s3c64xx/smdk6410_wm8987.o      

obj-m        += s3c_snd.o
s3c_snd-objs += soc-core.o soc-dapm.o          \
                s3c-pcm.o                      \
                s3c64xx/s3c6410-i2s-v32.o      \
                codecs/tlv320aic23.o           \
#                codecs/wm8987.o                \


EXTRA_CFLAGS += -I$(PWD)/../../inc #-D__DEBUG 

all:
	(cd $(KERNEL_DIR); make SUBDIRS=$(PWD) EXTRA_CFLAGS="${EXTRA_CFLAGS}" modules) 

clean:
	(for i in `find . -type d`; do (cd $$i && rm -rf *.o *.ko *.mod.c .*.o.flags .*.cmd *~); done)
	-rm -rf .tmp*
