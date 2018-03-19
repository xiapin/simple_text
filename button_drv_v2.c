#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm-generic/ioctl.h>

struct button_event{
    int code;               //键值
    int status;              //状态
};

struct s5pv210_button{
    struct class *cls;
    struct device *device;  //设备节点
    dev_t devno;            //存放动态申请的设备号
    struct cdev *cdev;      //包含操作方法及设备号的结构体
    unsigned int irqno;
    wait_queue_head_t wq_head;

    struct button_event event;
    int have_data;
};

struct s5pv210_button *button_dev;

/****************************中断处理函数**********************/
irqreturn_t button_irq_svc(int irqno, void *dev)
{
    int values;
   
    printk("\t\t\t****************%s***************\n",__FUNCTION__);

    switch (irqno){
        case IRQ_EINT(0):/*上*/
                values = gpio_get_value(S5PV210_GPH0(0));
                if(values){/*松开*/
                printk("kernel-->upkey--undown");
                button_dev->event.code = KEY_UP;
                button_dev->event.status = 0;
                 }
                else{
                printk("kernel-->upkey--down");
                button_dev->event.code = KEY_UP;
                button_dev->event.status = 1;
                }
            break;
        case IRQ_EINT(1):/*下*/
                values = gpio_get_value(S5PV210_GPH0(1));
                if(values){/*松开*/
                printk("kernel-->downkey--undown");
                button_dev->event.code = KEY_DOWN;
                button_dev->event.status = 0;
                 }
                else{
                printk("kernel-->downkey--down");
                button_dev->event.code = KEY_DOWN;
                button_dev->event.status = 1;
                }
            break;
        case IRQ_EINT(2):/*左*/
                values = gpio_get_value(S5PV210_GPH0(2));
                if(values){/*松开*/
                printk("kernel-->leftkey--undown");
                button_dev->event.code = KEY_LEFT;
                button_dev->event.status = 0;
                 }
                else{
                printk("kernel-->leftkey--down");
                button_dev->event.code = KEY_LEFT;
                button_dev->event.status = 1;
                }
            break;
        case IRQ_EINT(3):/*右*/
                values = gpio_get_value(S5PV210_GPH0(3));
                if(values){/*松开*/
                printk("kernel-->rightkey--undown");
                button_dev->event.code = KEY_RIGHT;
                button_dev->event.status = 0;
                 }
                else{
                printk("kernel-->rightkey--down");
                button_dev->event.code = KEY_RIGHT;
                button_dev->event.status = 1;
                }
            break;
            
        default:
            printk("unknow error\n");
                break;
    }

    /*******发生中断，有数据可读*******/
    button_dev->have_data = 1;
    /*************唤醒阻塞的进程*******/
    wake_up_interruptible(&button_dev->wq_head);

    return IRQ_HANDLED;
}

ssize_t button_read(struct file *flip, char __user *buf, size_t size, loff_t *flags)
{
    int ret;
    printk("\t\t\t****************%s***************\n",__FUNCTION__);
    /*如果没有数据，调用了非阻塞open,直接返回*/
    if(flip->f_flags & O_NONBLOCK && !button_dev->have_data)
        return -EAGAIN;

    /*①睡眠等待有资源可读*/
    wait_event_interruptible(button_dev->wq_head, button_dev->have_data);
    
    /*②将数据传给应用空间*/
    ret = copy_to_user(buf,&button_dev->event, size);
    if(ret > 0){
        printk("copy_to_user error\n");
        return EFAULT;
    }
    
    /*③清空按键状态，同时设置无数据可读*/
    memset(&button_dev->event, 0, sizeof(button_dev->event));
    button_dev->have_data = 0;
    return size;
}

/****************************操作方法****************************/
static struct file_operations fops = {
    .read = button_read,
};

/****************************模块加载函数**************************/

static int __init button_drv_init(void)
{
    int ret = -1;
    printk("\t\t\t****************%s***************\n",__FUNCTION__);
/*1.实例化模块对象*/
    button_dev = kzalloc(sizeof(struct s5pv210_button), GFP_KERNEL);
    if(IS_ERR(button_dev)){
        printk("kzalloc error\n");
        ret = PTR_ERR(button_dev);
        return -ENOMEM;
    }

/*2.申请设备号*/
    /*①获得设备号*/
    ret = alloc_chrdev_region(&button_dev->devno,0,1,"button_dev_up");/*上键*/
    if(ret < 0){
        printk("alloc_chrdev_region error\n");
        goto err_kfree;
    }
   ret = alloc_chrdev_region(&button_dev->devno,1,1,"button_dev_down");/*下键*/
   if(ret < 0){
        printk("alloc_chrdev_region error\n");
        goto err_kfree;
    }
   ret = alloc_chrdev_region(&button_dev->devno,2,1,"button_dev_left");/*左键*/
   if(ret < 0){
        printk("alloc_chrdev_region error\n");
        goto err_kfree;
    }
   ret = alloc_chrdev_region(&button_dev->devno,3,1,"button_dev_right");/*右键*/
   if(ret < 0){
        printk("alloc_chrdev_region error\n");
        goto err_kfree;
    }
    /*②创建cdev*/
    button_dev->cdev = cdev_alloc();
    if(IS_ERR(button_dev->cdev)){
        printk("cdev_alloc error\n");
        ret = PTR_ERR(button_dev->cdev);
        goto err_kfree;
    }
    cdev_init(button_dev->cdev, &fops);
    cdev_add(button_dev->cdev,button_dev->devno, 4);/*加入内核*/
    
/*3.创建设备节点*/
    /*①创建一个class结构体*/
    button_dev->cls = class_create(THIS_MODULE,"button class");
    if(IS_ERR(button_dev->cls)){
        printk("class_create error\n");
        ret = PTR_ERR(button_dev->cls);
        goto err_unregister;
    }
    /*②创建device对象*/
    button_dev->device = device_create(button_dev->cls,NULL,button_dev->devno,NULL,"button");
    if(IS_ERR(button_dev->device)){
        printk("device_create error\n");
        ret = PTR_ERR(button_dev->device);
        goto err_class;
    }

/*4.硬件初始化---申请中断*/
    /*①设置中断号*/
    button_dev->irqno = IRQ_EINT(0);/*上键*/
    /*②中断申请*/
    ret = request_irq(button_dev->irqno,button_irq_svc,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"key up",NULL);
    if(ret != 0){
        printk("request_irq error\n");
        goto err_device;
    }

    button_dev->irqno = IRQ_EINT(1);/*下键*/
    ret = request_irq(button_dev->irqno,button_irq_svc,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"key down",NULL);
    if(ret != 0){
        printk("request_irq error\n");
        goto err_device;
    }

    button_dev->irqno = IRQ_EINT(2);/*左键*/
    ret = request_irq(button_dev->irqno,button_irq_svc,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"key left",NULL);
    if(ret != 0){
        printk("request_irq error\n");
        goto err_device;
    }
    
    button_dev->irqno = IRQ_EINT(3);/*右键*/
    ret = request_irq(button_dev->irqno,button_irq_svc,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"key right",NULL);
    if(ret != 0){
        printk("request_irq error\n");
        goto err_device;
    }
    
    
    /*③初始化等待队列头*/
    init_waitqueue_head(&button_dev->wq_head);
    
    return 0;
/*5.异常处理*/
err_device:
    device_destroy(button_dev->cls, button_dev->devno);
err_class:
    class_destroy(button_dev->cls);
err_unregister:
    unregister_chrdev_region(button_dev->devno,1);
err_kfree:
    kfree(button_dev);
    
    return ret;
    
}

/****************************模块卸载函数**************************/

static void __exit button_drv_exit(void)
{
     printk("\t\t\t****************%s***************\n",__FUNCTION__);
     device_destroy(button_dev->cls, button_dev->devno);
     class_destroy(button_dev->cls);
     unregister_chrdev_region(button_dev->devno,1);
     kfree(button_dev);
}

/****************************模块申明注册**************************/

MODULE_DESCRIPTION("Driver for the key");
module_init(button_drv_init);
module_exit(button_drv_exit);
MODULE_LICENSE("GPL");





