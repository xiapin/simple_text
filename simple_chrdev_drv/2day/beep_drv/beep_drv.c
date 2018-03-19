#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>

/**************定义beep物理地址**************************************/
unsigned long *gpd0_conf;
unsigned long *gpd0_data;

/**************定义BEEP驱动类型**************************************/
struct s5pv210_beep{
    unsigned int major;
    struct class *cls;
    struct device *device;
};
struct s5pv210_beep *beep_dev;

/***********************定义flie_OPRT结构体*************************/
int beep_open(struct inode *inode, struct file * filp)
{
    printk("*************%s**************\n",__FUNCTION__);

    *gpd0_conf &= ~0xf0;
    *gpd0_conf |= 0x10;
    
    return 0;
}

int beep_close(struct inode * inode, struct file * file)
{
    printk("**********%s***********\n",__FUNCTION__);
    *gpd0_data &= ~0x10;
    return 0;
}

ssize_t beep_write(struct file *filp, const char __user *buf, size_t size, loff_t *flags)
{
    int data,ret;
    printk("**********%s***********\n",__FUNCTION__);

    ret = copy_from_user(&data,buf, size);
    if(ret > 0){
            printk("copy_from_user error");
            return -EFAULT;
    }

    if(data){
        *gpd0_data |= 0x2;
    }else{
        *gpd0_data &= ~(0x1<<1);
    }
    
    return size;
}
static struct file_operations fops = {
    .open = beep_open,
    .release = beep_close,
    .write = beep_write,
};

/************************模块加载函数********************************/
static int __init beep_drv_init(void)
{
    int ret;
     printk("**********%s***********\n",__FUNCTION__);
    /*0.驱动类型实例化*/
    
    beep_dev = kzalloc(sizeof(struct s5pv210_beep), GFP_KERNEL);
    if(IS_ERR(beep_dev)){
           printk(KERN_ERR"kzalloc error\n");
           ret = PTR_ERR(beep_dev);
           return -ENOMEM;
    }
    
    /*1.申请设备号*/
    beep_dev->major = register_chrdev(0,"beep_drv", &fops);
    if(beep_dev->major < 0){
            printk(KERN_ERR"register_chrdev error\n");
            return -EINVAL;
            goto err_kfree;
    }
    
    /*2.创建设备节点*/
    beep_dev->cls = class_create(THIS_MODULE,"beep_class");
    if(IS_ERR(beep_dev->cls)){
            printk("class_create error\n");
            ret = PTR_ERR(beep_dev->cls);
            goto err_unregister;
    }

    beep_dev->device = device_create(beep_dev->cls,NULL,MKDEV(beep_dev->major,0),NULL,"BEEP");
    if(IS_ERR(beep_dev->device)){
             printk(KERN_ERR"device_create error\n");
             return PTR_ERR(beep_dev->device);
             goto err_class;
    }
    
    /*3.硬件初始化，地址映射*/
    gpd0_conf = ioremap(0xe02000a0,8);
    gpd0_data = gpd0_conf + 1;
  
  return 0;
    /*4.出错处理*/
err_class:
    class_destroy(beep_dev->cls);
err_unregister:
    unregister_chrdev(beep_dev->major, "beep_dev");
err_kfree:
    kfree(beep_dev);
    
   return ret; 
    
}


/*************************模块卸载函数*******************************/

static void __exit beep_drv_exit(void)
{
    printk("**********%s***********\n",__FUNCTION__);

    device_destroy(beep_dev->cls,MKDEV(beep_dev->major, 0));
    class_destroy(beep_dev->cls);
    unregister_chrdev(beep_dev->major, "beep_dev");
    kfree(beep_dev);
}

MODULE_AUTHOR("PIN_XIA@163.com");
MODULE_DESCRIPTION("BEEP DRIVER");

module_init(beep_drv_init);
module_exit(beep_drv_exit);
MODULE_LICENSE("GPL");





























