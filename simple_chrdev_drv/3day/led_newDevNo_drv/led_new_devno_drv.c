//ͷ�ļ�
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
 
/*******************����һ���豸��������**********************/
struct s5pv210_led{
    unsigned int major;
    struct class *cls;
    struct device *device;
    struct cdev *cdev;
    dev_t devno;
};

/*************************��������ʵ����************************/
struct s5pv210_led *led_dev;

/**************************�ں˲������ַ************************/
unsigned long *gpc0_conf;/*���ƼĴ�����ַ*/
unsigned long *gpc0_data;/*���ݼĴ�����ַ*/

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

    if(data){/*���*/
        *gpc0_data &= 0xe7;
        *gpc0_data |= 0x18;
    }else{/*���*/
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

/**************************�ṩ���ϲ�Ľӿ�************************/
static struct file_operations fops = {
	.open = led_open,/*�����豸ʱ����*/
    .release = led_close,
    .write = led_write,
    .unlocked_ioctl = led_ioctl,
};

/*************************ģ����غ���**************************/
static int __init led_drv_init(void)
{
	int ret;
	printk("--------------%s--------------\n",__FUNCTION__);
/*********************0.ʵ��������******************************/
led_dev = kzalloc(sizeof(struct s5pv210_led),GFP_KERNEL);
if(IS_ERR(led_dev)){
    printk("kzalloc error\n");
    ret = PTR_ERR(led_dev);

    return -ENOMEM;
}

/**********************1.�����豸��*****************************/
#if 0
/*��̬����1*/
	ret = register_chrdev(led_major,"led_major",&fops);
	if(ret < 0){
		printk("register_chrdev error\n");
		return -EINVAL;
	}
/*��̬����1*/
	led_dev->major = register_chrdev(0,"led_major",&fops);
	if(led_dev->major < 0){
	    printk("register_chrdev error\n");
	    return -EINVAL;
	}
/*��̬����2*/
    led_dev->major = 256;
    ret = register_chrdev_region(MKDEV(led_dev->major, 0), 1, "led_dev");
    if(ret < 0 ){
        printk("register_chrdev_region error\n");
        goto err_kfree;
    }
#else
/*��̬����2*/
    /*1.�豸�Ŵ�ŵĵ�ַ 2.���豸�� 3.�������� 4.��ǩ*/
    ret = alloc_chrdev_region(&led_dev->devno,0, 1, "led_dev");
    if(ret < 0){
        printk("alloc_chrdev_region error\n");
        goto err_kfree;
    }
/*����cdev*/
    led_dev->cd ev = cdev_alloc();
    if(IS_ERR(led_dev->cdev)){
        printk("cdev_alloc error\n");
        ret = PTR_ERR(led_dev->cdev);
        goto err_unregister;
    }
    cdev_init(led_dev->cdev, &fops);
    cdev_add(led_dev->cdev,led_dev->devno,1);
#endif	

/**************************2.�Զ������豸�ڵ�*******************/
  led_dev->cls = class_create(THIS_MODULE,"led_class");
   if(IS_ERR(led_dev->cls)){
	    printk("led_cls error\n");
        /************�����***************/
        ret = PTR_ERR(led_dev->cls);
        
	    goto err_unregister;//ʧ����ж������豸��
	}

   led_dev->device = device_create(led_dev->cls,NULL,led_dev->devno, NULL, "led%d",2);
   if(IS_ERR(led_dev->device )){
        printk("led_device error\n");
        ret = PTR_ERR(led_dev->device );

        goto err_class;
   }

   
/**************************3.Ӳ����ʼ��***************************/ 
#if 0
    gpc0_conf = ioremap(0xe0200060,4);/*1.Ӳ�������ַ��2.ӳ�䳤��*/
    gpc0_data = ioremap(0xe0200064,4);
#else
    gpc0_conf = ioremap(0xe0200060,8);
    gpc0_data = gpc0_conf+1;/*gpc0cof�ĳ�ʼ��ַƫ��4���ֽ�*/
#endif


	return 0;

/***************************������*******************/
err_class:
    class_destroy(led_dev->cls);
err_unregister:
    unregister_chrdev_region(led_dev->devno, 1);
err_kfree:
    kfree(led_dev);
 
return ret;
}


/***********************ģ��ж��*****************************/
/***********************���⵽��*****************************/
static void __exit led_drv_exit(void)
{
	printk("--------------%s--------------\n",__FUNCTION__);

    device_destroy(led_dev->cls, led_dev->devno);
    class_destroy(led_dev->cls);
	unregister_chrdev_region(led_dev->devno, 1);
    
    kfree(led_dev);
}

//ģ����������֤
module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
