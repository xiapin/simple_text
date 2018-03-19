//ͷ�ļ�
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>

/*******************����һ���豸��������**********************/
struct s5pv210_hello{
    unsigned int major;
    struct class *cls;
    struct device *device;
};

/*************************��������ʵ����************************/
struct s5pv210_hello *hello_dev;

 int hello_open(struct inode *inode,struct file * filp){
      printk("---------------%s-----------------\n",__FUNCTION__);
      return 0;
}

static struct file_operations fops = {
	.open = hello_open,
};

/*************************ģ����غ���**************************/
static int __init hello_drv_init(void)
{
	int ret;
	printk("--------------%s--------------\n",__FUNCTION__);
/*********************0.ʵ��������******************************/
hello_dev = kzalloc(sizeof(struct s5pv210_hello),GFP_KERNEL);
if(IS_ERR(hello_dev)){
    printk("kzalloc error\n");
    ret = PTR_ERR(hello_dev);

    return -ENOMEM;
}

/**********************1.�����豸��*****************************/
#if 0
	ret = register_chrdev(hello_major,"hello_major",&fops);
	if(ret < 0){
		printk("register_chrdev error\n");
		return -EINVAL;
	}
#else
	hello_dev->major = register_chrdev(0,"hello_major",&fops);
	if(hello_dev->major < 0){
	    printk("register_chrdev error\n");
	    return -EINVAL;
	}
#endif	

/**************************2.�Զ������豸�ڵ�*******************/
  hello_dev->cls = class_create(THIS_MODULE,"hello_class");
   if(IS_ERR(hello_dev->cls)){
	    printk("hello_cls error\n");
        /************�����***************/
        ret = PTR_ERR(hello_dev->cls);
        
	    goto err_unregister;//ʧ����ж������豸��
	}

   hello_dev->device = device_create(hello_dev->cls,NULL,MKDEV(hello_dev->major, 0), NULL, "hello%d",2);
   if(IS_ERR(hello_dev->device )){
        printk("hello_device error\n");
        ret = PTR_ERR(hello_dev->device );

        goto err_class;
   }
    
   
	return 0;

err_class:
    class_destroy(hello_dev->cls);
err_unregister:
    unregister_chrdev(hello_dev->major, "hello_drv");
kfree(hello_dev);
 
return ret;
}


/***********************ģ��ж��*****************************/
/***********************���⵽��*****************************/
static void __exit hello_drv_exit(void)
{
	printk("--------------%s--------------\n",__FUNCTION__);

    device_destroy(hello_dev->cls, MKDEV(hello_dev->major, 0));
    class_destroy(hello_dev->cls);
	unregister_chrdev(hello_dev->major,"hello_dev->drv");
    
    kfree(hello_dev);
}

//ģ����������֤
module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");
