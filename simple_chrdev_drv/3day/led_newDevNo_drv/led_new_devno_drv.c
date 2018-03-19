//头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/cdev.h>

#include <asm-generic/ioctl.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#define LED_ALL_ON _IO('L', 0x1111)
#define LED_ALL_OFF _IO('L', 0x2222)
#define LED_NUM_ON _IOW('L', 0x1234, int)
#define LED_NUM_OFF _IOW('L', 0x6789, int)
 
/*******************定义一个设备驱动类型**********************/
struct s5pv210_led{
    unsigned int major;
    struct class *cls;
    struct device *device;
    struct cdev *cdev;
    dev_t devno;
};

/*************************驱动类型实例化************************/
struct s5pv210_led *led_dev;

/**************************内核层物理地址************************/
unsigned long *gpc0_conf;/*控制寄存器地址*/
unsigned long *gpc0_data;/*数据寄存器地址*/

int led_open(struct inode *inode,struct file * filp){
      printk("---------------%s-----------------\n",__FUNCTION__);

      *gpc0_conf &= ~(0xff<<12);
      *gpc0_conf |= 0x11<<12;
      return 0;
}
int led_close(struct inode *innode, struct file *flip){

    printk("****************%s***************\n",__FUNCTION__);
    *gpc0_data &= 0xe7;
    return 0;
}
ssize_t led_write(struct file * file, const char __user *buf, size_t size, loff_t *flags){
    int data,ret;
    ret = copy_from_user(&data,buf,size);
    if(ret > 0){
        printk(KERN_ERR"copy_from_usr error!\n");
        return -EFAULT;
    }

    if(data){/*点灯*/
        *gpc0_data &= 0xe7;
        *gpc0_data |= 0x18;
    }else{/*灭灯*/
        *gpc0_data &= 0xe7;
    }
    
    return 0;
}

long led_ioctl (struct file *flip, unsigned int cmd, unsigned long args)
{
    unsigned long num = args + 2;
    printk("****************%s***************\n",__FUNCTION__);
    switch(cmd){
        case LED_NUM_ON:
            if(num != 3 && num != 4)
                return -EINVAL;
            
            //*gpc0_data |= 0x1<<num;
            gpio_request(S5PV210_GPC0(num),"gpio_led");
            gpio_direction_output(S5PV210_GPC0(num),1);
            gpio_free(S5PV210_GPC0(num));
            break;
        case LED_NUM_OFF:
            if(num != 3 && num != 4)
                return -EINVAL;
            //*gpc0_data &= ~(0x1<<num);
            gpio_request(S5PV210_GPC0(num),"gpio_led");
            gpio_direction_output(S5PV210_GPC0(num),0);
            gpio_free(S5PV210_GPC0(num));
            break;
        case LED_ALL_ON:
             //*gpc0_data &= 0xe7;
             //*gpc0_data |= 0x18;
             gpio_request(S5PV210_GPC0(3),"gpio_led");
             gpio_direction_output(S5PV210_GPC0(3),1);
             gpio_free(S5PV210_GPC0(3));
             gpio_request(S5PV210_GPC0(4),"gpio_led");
             gpio_direction_output(S5PV210_GPC0(4),1);
             gpio_free(S5PV210_GPC0(4));
            break;
        case LED_ALL_OFF:
            //*gpc0_data &= 0xe7;
             gpio_request(S5PV210_GPC0(3),"gpio_led");
             gpio_direction_output(S5PV210_GPC0(3),0);
             gpio_free(S5PV210_GPC0(3));
             gpio_request(S5PV210_GPC0(4),"gpio_led");
             gpio_direction_output(S5PV210_GPC0(4),0);
             gpio_free(S5PV210_GPC0(4));
            break;
        default:
            printk("enter error\n");
            break;
    }
    return 0;
}

/**************************提供给上层的接口************************/
static struct file_operations fops = {
	.open = led_open,/*加载设备时调用*/
    .release = led_close,
    .write = led_write,
    .unlocked_ioctl = led_ioctl,
};

/*************************模块加载函数**************************/
static int __init led_drv_init(void)
{
	int ret;
	printk("--------------%s--------------\n",__FUNCTION__);
/*********************0.实例化对象******************************/
led_dev = kzalloc(sizeof(struct s5pv210_led),GFP_KERNEL);
if(IS_ERR(led_dev)){
    printk("kzalloc error\n");
    ret = PTR_ERR(led_dev);

    return -ENOMEM;
}

/**********************1.申请设备号*****************************/
#if 0
/*静态方法1*/
	ret = register_chrdev(led_major,"led_major",&fops);
	if(ret < 0){
		printk("register_chrdev error\n");
		return -EINVAL;
	}
/*动态方法1*/
	led_dev->major = register_chrdev(0,"led_major",&fops);
	if(led_dev->major < 0){
	    printk("register_chrdev error\n");
	    return -EINVAL;
	}
/*静态方法2*/
    led_dev->major = 256;
    ret = register_chrdev_region(MKDEV(led_dev->major, 0), 1, "led_dev");
    if(ret < 0 ){
        printk("register_chrdev_region error\n");
        goto err_kfree;
    }
#else
/*动态方法2*/
    /*1.设备号存放的地址 2.次设备号 3.创建个数 4.标签*/
    ret = alloc_chrdev_region(&led_dev->devno,0, 1, "led_dev");
    if(ret < 0){
        printk("alloc_chrdev_region error\n");
        goto err_kfree;
    }
/*创建cdev*/
    led_dev->cd ev = cdev_alloc();
    if(IS_ERR(led_dev->cdev)){
        printk("cdev_alloc error\n");
        ret = PTR_ERR(led_dev->cdev);
        goto err_unregister;
    }
    cdev_init(led_dev->cdev, &fops);
    cdev_add(led_dev->cdev,led_dev->devno,1);
#endif	

/**************************2.自动挂载设备节点*******************/
  led_dev->cls = class_create(THIS_MODULE,"led_class");
   if(IS_ERR(led_dev->cls)){
	    printk("led_cls error\n");
        /************错误宏***************/
        ret = PTR_ERR(led_dev->cls);
        
	    goto err_unregister;//失败则卸载清除设备号
	}

   led_dev->device = device_create(led_dev->cls,NULL,led_dev->devno, NULL, "led%d",2);
   if(IS_ERR(led_dev->device )){
        printk("led_device error\n");
        ret = PTR_ERR(led_dev->device );

        goto err_class;
   }

   
/**************************3.硬件初始化***************************/ 
#if 0
    gpc0_conf = ioremap(0xe0200060,4);/*1.硬件物理地址，2.映射长度*/
    gpc0_data = ioremap(0xe0200064,4);
#else
    gpc0_conf = ioremap(0xe0200060,8);
    gpc0_data = gpc0_conf+1;/*gpc0cof的初始地址偏移4个字节*/
#endif


	return 0;

/***************************出错处理*******************/
err_class:
    class_destroy(led_dev->cls);
err_unregister:
    unregister_chrdev_region(led_dev->devno, 1);
err_kfree:
    kfree(led_dev);
 
return ret;
}


/***********************模块卸载*****************************/
/***********************由外到内*****************************/
static void __exit led_drv_exit(void)
{
	printk("--------------%s--------------\n",__FUNCTION__);

    device_destroy(led_dev->cls, led_dev->devno);
    class_destroy(led_dev->cls);
	unregister_chrdev_region(led_dev->devno, 1);
    
    kfree(led_dev);
}

//模块申明和认证
module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
