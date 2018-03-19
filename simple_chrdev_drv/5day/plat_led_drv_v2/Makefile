
CROSS_COMPILE = arm-none-linux-gnueabi-
CC = $(CROSS_COMPILE)gcc
APP_NAME = led_app
MODULE_NAME = plat_led_dev_v2
MODULE_NAME2 = plat_led_drv_v2


#内核源码路径
KERNEL_DIR = /home/farsight/s5pv210/kernel/linux-3.0.8 

CUR_DIR = $(shell pwd)

all :
	    make -C $(KERNEL_DIR) M=$(CUR_DIR) modules 
	    $(CC) $(APP_NAME).c -o  $(APP_NAME)

clean : 
	    make -C $(KERNEL_DIR) M=$(CUR_DIR) clean
	     rm -rf $(APP_NAME)	

install:
	    cp -raf  *.ko  $(APP_NAME) /opt/rootfs/drv_module/


#指定编译哪个源文件
obj-m = $(MODULE_NAME).o
obj-m += $(MODULE_NAME2).o



