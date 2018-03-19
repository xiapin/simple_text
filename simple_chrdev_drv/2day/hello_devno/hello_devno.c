//头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

static unsigned int hello_major = 256;

 int hello_open(struct inode *inode,struct file * filp){
      printk("---------------%s-----------------\n",__FUNCTION__);
      return 0;
}

static struct file_operations fops = {
	.open = hello_open,
};


//模块加载函数
static int __init hello_drv_init(void)
{
	int ret;
	printk("--------------%s--------------\n",__FUNCTION__);

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
	return 0;
}
//模块卸载函数
static void __exit hello_drv_exit(void)
{
	printk("--------------%s--------------\n",__FUNCTION__);
	unregister_chrdev(hello_major,"hello_drv");
}

//模块声明和认证
module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");
