#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>

volatile unsigned long *gpc0_conf;
volatile unsigned long *gpc0_data;

struct s5pv210_led{
    int dev_major;
    struct class *cls;
    struct device *dev;
    int value;
    void *reg_base;//gpc0_conf起始地址
};

struct s5pv210_led *led_dev; 
    
#define GPC0_DATA 4

int led_drv_open(struct inode *inode, struct file *flip)
{
    unsigned long value;
    
    printk("--------------%s-------------\n",__FUNCTION__);

    value = __raw_readl(led_dev->reg_base);
    value &= ~(0xff<<12);
    value |= 0x11<<12;

    __raw_writel(value, led_dev->reg_base);
    
    return 0;
}
ssize_t led_drv_write(struct file *flip, const char __user *user, size_t size, loff_t *fpos)
{
    int ret = -1;
    printk("--------------%s-------------\n",__FUNCTION__);

    ret = copy_from_user(&led_dev->value,user,size);
    if(ret > 0){
        printk("copy_from_user error\n");
        return -EFAULT;
    }

    if(led_dev->value){//点灯
           writel(readl(led_dev->reg_base + GPC0_DATA) | 0x18,led_dev->reg_base + GPC0_DATA);
    }else{
           writel(readl(led_dev->reg_base + GPC0_DATA) &(~0x18),led_dev->reg_base + GPC0_DATA);
    }
    
    return size;
}
int led_drv_close(struct inode *inode, struct file *flip)
{
    printk("--------------%s-------------\n",__FUNCTION__);
    writel(readl(led_dev->reg_base + GPC0_DATA) &(~0x18),led_dev->reg_base + GPC0_DATA);
    return 0;
}

const struct file_operations led_fops = {
    .open = led_drv_open,
    .write = led_drv_write,
    .release = led_drv_close,
};

/*匹配时加载的函数，获取cdev中的资源*/
int led_drv_probe(struct platform_device *pdev)
{
    struct resource *addr_res;
    int ret = -1;
    printk("--------------%s-------------\n",__FUNCTION__);
    /*
    *编写字符驱动套路
    */
    /*0.实例化led_dev对象*/
    led_dev = kzalloc(sizeof(struct s5pv210_led),GFP_KERNEL);
    if(led_dev == NULL){
        printk("kazlloc error\n");
        return -ENOMEM;
    }
    printk("0");
    /*1.申请设备号*/
    led_dev->dev_major = register_chrdev(0,"led_dev",&led_fops);
    if(led_dev < 0){
        printk("register_chrdev error\n");
        ret = -EINVAL;
        goto err_free;
    }
    printk("1");
    /*2.创建设备节点*/
    led_dev->cls = class_create(THIS_MODULE,"led_cls");
    if(IS_ERR(led_dev->cls)){
        printk("class_create error\n");
        ret = PTR_ERR(led_dev->cls);
        goto err_unregister;
    } 
    printk("2");

    led_dev->dev = device_create(led_dev->cls, NULL,MKDEV(led_dev->dev_major, 0),NULL, "plat_led");
    if(IS_ERR(led_dev->dev)){
         printk("device_create error\n");
         ret = PTR_ERR(led_dev->dev);
         goto err_class_destroy;
    }
    /*获取cdev中的资源进行初始化*/
    /*获取参数2所指类型中的第参数3个资源*/
    /*硬件初始化*/
    addr_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(addr_res == NULL){
        printk("platform_get_resource error\n");
        ret  = PTR_ERR(addr_res);
        goto err_device_create;
    }
    printk("3");
    
    led_dev->reg_base= ioremap(addr_res->start, resource_size(addr_res));
    if(IS_ERR(led_dev->reg_base)){
           printk("ioremap error\n");
           ret = PTR_ERR(led_dev->reg_base);
           goto err_device_create;
    }

    printk("4");
#if 0 
     int irqno = platform_get_irq(pdev, 0);

#endif
    return 0;

/*出错处理*/
err_device_create:
	 device_destroy(led_dev->cls, MKDEV(led_dev->dev_major, 0));

err_class_destroy:
        class_destroy(led_dev->cls);

err_unregister:
        unregister_chrdev(led_dev->dev_major, "led_drv");

err_free:
        kfree(led_dev);
        return ret;

}
int led_drv_remove(struct platform_device *pdev)
{
    device_destroy(led_dev->cls, MKDEV(led_dev->dev_major, 0));
    class_destroy(led_dev->cls);
    unregister_chrdev(led_dev->dev_major, "led_drv");
     kfree(led_dev);

    return 0;
}
    
const struct platform_device_id led_id_table[] = {
        {"s5pv210_led",0x1111},
        {"s3c2410_led",0x2222},
};

/*定义一个pdrv类型*/
struct platform_driver led_pdrv = {
    .probe = led_drv_probe,
    .remove = led_drv_remove,
    .driver = {
        .name = "s5pv210_led", /*一定要有*/
    },
    .id_table = led_id_table,/*用于匹配*/
};

static int __init plat_led_drv_init(void)
{
    printk("--------------%s-------------\n",__FUNCTION__);
    /*注册一个pdrv*/
    return platform_driver_register(&led_pdrv);
    
}
static void __exit plat_led_drv_exit(void)
{
    printk("--------------%s-------------\n",__FUNCTION__);
    /*注销一个pdrv*/
    platform_driver_unregister(&led_pdrv);  
}

module_init(plat_led_drv_init);
module_exit(plat_led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LAOXIA@169.com");


