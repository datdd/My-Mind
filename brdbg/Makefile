KERNELDIR=$(KERNEL_DIR)
EXTRA_CFLAGS= -Wall -Werror

BRDBG_OBJ = brdbg.o
obj-m = $(BRDBG_OBJ)

all:
	$(MAKE) $(EXTRA_CFLAGS) -C $(KERNELDIR) M=`pwd` modules
	$(STRIP) --strip-unneeded *.ko
	
clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
