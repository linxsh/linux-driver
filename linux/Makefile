# Common flags to be passed for driver compilation
EXTRA_CFLAGS += -Winline -Wall -I${DR_SRC_PATH}/src/common -I${DR_SRC_PATH}/include -funsigned-char
EXTRA_CFLAGS += -D$(DR_ARCH)_$(DR_OS) -DUSE_IMMEDIATE

obj-m :=  $(DR_MODULE).o

include ${DR_SRC_PATH}/linux/module.mk
include $(DR_SRC_PATH)/scripts/$(DR_ARCH)-$(DR_OS)-compiler.mak

$(DR_MODULE)-objs = $(base-objs)

default: all

$(DR_MODULE):  $($(DR_MODULE)-objs)
	$(LD) -r $($(DR_MODULE)-objs) -o $(DR_MODULE).o
	rm $(DR_MODULE).o
all:
	$(MAKE) ARCH=$(DR_ARCH) CROSS_COMPILE=$(DR_CROSS_COMPILE) -C $(DR_KERNEL_PATH) SUBDIRS=`pwd`

clean:
	rm -rf $(base-objs)        \
		.*.cmd *.mod.c *.o *.ko .tmp_versions Module.symvers       \
		modules.order tags .$(DR_MODULE).o.cmd .tmp_versions .ioctl.o.d
	find ../ -name "*.cmd" -delete
