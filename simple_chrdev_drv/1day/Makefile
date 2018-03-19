KERNEL_DIR=/home/farsight/s5pv210/kernel/linux-3.0.8
CUR_DIR=$(shell pwd)


all:
	    make -C $(KERNEL_DIR) M=$(CUR_DIR) modules
clean:
	    make -C $(KERNEL_DIR) M=$(CUR_DIR) clean

install:
	    cp *.ko /opt/rootfs/drv_module/
obj-m=hello_drv.o 
