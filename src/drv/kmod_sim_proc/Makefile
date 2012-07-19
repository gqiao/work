KERNEL_DIR=/work/prj/teclong/v200/s3c6410/linux-2.6
MAKE_ENV=ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}

NAME=kmod_sim_proc
TARGET = $(NAME).ko

HOST_INSTALL_DIR  = .
obj-m            += $(NAME).o
$(NAME)-objs     := sim_proc.o 

EXTRA_CFLAGS     += #-DUSE_UDEV  #-march=armv7a -mfpu=neon 
EXTRA_AFLAGS     += #-march=armv7a -mfpu=neon 

debug: EXTRA_CFLAGS += -D__DEBUG 

.PHONY: clean debug release install default

default: release

docs:

release:
	make -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules
	# @mv $(TARGET) $(HOST_INSTALL_DIR)/

debug:
	make -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules
	# @mv $(TARGET) $(HOST_INSTALL_DIR)/

install:
	install -d $(HOST_INSTALL_DIR)
	install -c $(TARGET) $(HOST_INSTALL_DIR)

clean:
	-rm -rf *.o *.ko .tmp* *~ Module.symvers *.mod.c .*o.cmd  $(HOST_INSTALL_DIR)/$(TARGET)
