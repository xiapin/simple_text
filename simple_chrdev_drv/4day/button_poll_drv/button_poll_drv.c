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
#include <linux/poll.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm-generic/ioctl.h>

/*按键响应数据*/
struct button_event{
    int code;               //键值
    int status;              //状态
};

/*设备信息*/
struct s5pv210_button{
    struct class *cls;
    struct device *device;  //设备节点
    dev_t devno;            //存放动态申请的设备号
    struct cdev *cdev;      //包含操作方法及设备号的结构体
    unsigned int irqno;
    wait_queue_head_t wq_head;

    struct button_event event;
    int have_data;
    void *virt_mem;
};
/*按键信息*/
struct buttons{
    char *name;         
    unsigned int irqno; //中断号
    int gpio;           //管脚
    int code;           //键值
    unsigned long flags;//中断方式
};

struct buttons buttons_set[] = {
    [0] = {
        .name = "key_up",
        .irqno = IRQ_EINT(0),
        .gpio = S5PV210_GPH0(0),
        .code = KEY_UP,
        .flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
    [1] = {
        .name = "key_down",
        .irqno = IRQ_EINT(1),
        .gpio = S5PV210_GPH0(1),
        .code = KEY_DOWN,
        .flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
    [2] = {
        .name = "key_left",
        .irqno = IRQ_EINT(2),
        .gpio = S5PV210_GPH0(2),
        .code = KEY_LEFT,
        .flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
    [3] = {
        .name = "key_right",
        .irqno = IRQ_EINT(3),
        .gpio = S5PV210_GPH0(3),
        .code = KEY_RIGHT,
        .flags= IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
    
};
struct mem_data{
      char buf[128];
};

struct s5pv210_button *button_dev;

/****************************中断处理函数**********************/
irqreturn_t button_irq_svc(int irqno, void *dev)
{
    int values;
    struct buttons *p;
    printk("\t\t\t****************%s***************\n",__FUNCTION__);

    p = (struct buttons *)dev;
    
    values = gpio_get_value(p->gpio);
    if(values){/*松开*/
        printk("kernel-->%s--undown",p->name);
        button_dev->event.code = p->code;
        button_dev->event.status = 0;
    }else{
        printk("kernel-->%s--down",p->name);
        button_dev->event.code = p->code;
        button_dev->event.status = 1;
    }

    /*******发生中断，有数据可读*******/
    button_dev->have_data = 1;
    /*************唤醒阻塞的进程*******/
    wake_up_interruptible(&button_dev->wq_head);

    return IRQ_HANDLED;
}

/****************************阻塞读函数**********************/
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

/****************************轮询函数***************************/
unsigned int button_poll (struct file *flip, struct poll_table_struct *pts)
{   
        unsigned int mask = 0;
        printk("\t\t\t****************%s***************\n",__FUNCTION__);
        /*①将等待队列头注册到VFS系统*/
        poll_wait(flip,&button_dev->wq_head, pts);
        /*②如果有数据，返回POLLIN*/
        if(button_dev->have_data)
            mask |= POLLIN;
        
        return mask;
}

/****************************BUTTON_IOCTL**********************/
unsigned int button_ioctl(struct file *flip,unsigned int cmd,unsigned long args)
{
      void __USER *argp;
      int ret;
      struct mem_data data;
      printk("\t\t\t****************%s***************\n",__FUNCTION__);
      
      argp = (void __user*)args;
      switch(cmd){
	    case BUTTON_IOC_DATA:
		  mem_set(&data.buf,0,sizeof(data.buf));
		  memcpy(data.buf,button_dev->virt_mem,sizeof(data.buf));
		  ret = copy_to_user(argp,&data,sizeof(data));
		  if(ret > 0){
			printk("copy_to_user falied\n");
			return -EFAULT;
		  }
	    break;
      default:
	    printk("unknown cmd\n");	    
	    break;
      }

      return 0;
}

/****************************FUNCTION MMP***********************/
int button_mmp(struct file *flip,struct vm_area_struct *vma)
{
      phys_addr_t addr;
      printk("\t\t\t****************%s***************\n",__FUNCTION__);
      
      /*virtual space to physical storage*/
      addr = virt_to_phys(button_dev->virt_mem);
      /*physical space map tp application space*/
      vma->vm->flags |=VM_IO;
      vma->vm_page_port = pgprot_noncached(vma->vm_page_port);

      if(io_remap_pfn_range(vma,vma->vm_start,addr>>PAGE_SHIFT,PAGE_SIZE
			,vma->vm_page_port)){
	    printk(KER_ERR"%S:io_remap_fpn_range failed\n",__func__);
	    return -EAGAIN;
      }
      return 0;
}
/****************************操作方法****************************/
static struct file_operations fops = {
    .read = button_read,
    .poll = button_poll, 
    .unlocked_ioctl = button_ioctl,
    .mmp = button_mmp;
};

/****************************模块加载函数**************************/

static int __init button_drv_init(void)
{
    int ret = -1,i;
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
    ret = alloc_chrdev_region(&button_dev->devno,0,1,"button_dev");
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
    cdev_add(button_dev->cdev,button_dev->devno, 1);
    
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
    /*②中断申请*/
    for(i = 0;i < ARRAY_SIZE(buttons_set);i++)
    {
        ret = request_irq(buttons_set[i].irqno,button_irq_svc,buttons_set[i].flags,buttons_set[i].name,&buttons_set[i]);
        if(ret != 0){
            printk("request_irq error\n");
            goto err_device;
        }
    }
    /*③初始化等待队列头*/
    init_waitqueue_head(&button_dev->wq_head);

   /*apply for a virtual room ,and translate it to physical room by mmp*/
      button_dev->virt_mem = kzalloc(PAGE_SIZE,GFP_KERNEL);
      if(IS_ERR(button_dev->virt_mem)){
	    printk("kzalloc error\n");
	    ret = _ENOMEM;
	    goto err_free_irq;
      }   



    return 0;
/*5.异常处理*/
err_free_irq:
    for(i = 0;i < ARRAY_SIZE(button_set);i++)
	    free_irq(buttons_set[i].irqno,&buttos_set[i]);
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
     int i;
     printk("\t\t\t****************%s***************\n",__FUNCTION__);
     for(i = 0;i < ARRAY_SIZE(buttons_set);i++)
        free_irq(buttons_set[i].irqno, &button_dev->devno);
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





