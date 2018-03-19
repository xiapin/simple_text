#指定内核源码的绝对路径
KERNEL_DIR = /home/farsight/s5pv210/kernel/linux-3.0.8
CUR_DIR = $(shell pwd)
MYMODULE= beep_drv
MYAPP = beep

all:
		#make进入到内涵源码目录，将当前目录下的源程序作为内核模块一起编译
	    make -C $(KERNEL_DIR) M=$(CUR_DIR) modules
	    arm-none-linux-gnueabi-gcc -o $(MYAPP) $(MYAPP).c
install:
	    cp $(MYAPP) *.ko /opt/rootfs/drv_module
clean:
		#将编译生成的文件删除
		make -C $(KERNEL_DIR) M=$(CUR_DIR) clean
		  $(RM) $(MYAPP) .*.sw?
obj-m = $(MYMODULE).o
		

