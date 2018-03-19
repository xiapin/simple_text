
CROSS_COMPILE = arm-none-linux-gnueabi-
CC = $(CROSS_COMPILE)gcc
APP_NAME = at24_app
MODULE_NAME = i2c_at24_drv
#MODULE_NAME2 = plat_led_drv_v2


#内核源码路径
KERNEL_DIR = /home/farsight/s5pv210/kernel/linux-3.0.8 

CUR_DIR = $(shell pwd)

all :
	    make -C $(KERNEL_DIR) M=$(CUR_DIR) modules 

ifneq ($(APP_NAME), )	    
	    $(CC) $(APP_NAME).c -o  $(APP_NAME)
endif

clean : 
	    make -C $(KERNEL_DIR) M=$(CUR_DIR) clean
	     rm -rf $(APP_NAME)	

install:
	    cp -raf  *.ko  $(APP_NAME) /opt/rootfs/drv_module/


#指定编译哪个源文件
obj-m = $(MODULE_NAME).o
ifneq ($(MODULE_NAME2), )
obj-m += $(MODULE_NAME2).o
endif


