//ͷ�ļ�
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>

static unsigned int hello_major = 256;
static struct class *hello_cls;
static struct device *hello_device;

 int hello_open(struct inode *inode,struct file * filp){
      printk("---------------%s-----------------\n",__FUNCTION__);
      return 0;
}

static struct file_operations fops = {
	.open = hello_open,
};


//ģ����غ���
static int __init hello_drv_init(void)
{
	int ret;
	printk("--------------%s--------------\n",__FUNCTION__);

//1.�����豸��
#if 0
	ret = register_chrdev(hello_major,"hello_major",&fops);
	if(ret < 0){
		printk("register_chrdev error\n");
		return -EINVAL;
	}
#else
	hello_major = register_chrdev(0,"hello_major",&fops);
	if(hello_major < 0){
	    printk("register_chrdev error\n");
	    return -EINVAL;
	}
#endif	

//2.�Զ������豸�ڵ�
  hello_cls = class_create(THIS_MODULE,"hello_class");
   if(IS_ERR(hello_cls)){
	    printk("hello_cls error\n");
        /************�����***************/
        ret = PTR_ERR(hello_cls);
        
	    goto err_unregister;//ʧ����ж������豸��
	}

   hello_device = device_create(hello_cls,NULL,MKDEV(hello_major, 0), NULL, "hello%d",2);
   if(IS_ERR(hello_device)){
        printk("hello_device error\n");
        ret = PTR_ERR(hello_device);

        goto err_class;
   }
    
   
	return 0;

err_class:
    class_destroy(hello_cls);
err_unregister:
    unregister_chrdev(hello_major, "hello_drv");
return ret;
}


//ģ��ж�غ���
static void __exit hello_drv_exit(void)
{
	printk("--------------%s--------------\n",__FUNCTION__);
	unregister_chrdev(hello_major,"hello_drv");
}

//ģ����������֤
module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");
