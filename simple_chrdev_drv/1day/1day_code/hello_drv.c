//头文件
#include <linux/init.h>
#include <linux/module.h>

//模块加载函数
static int __init hello_drv_init(void)
{
	printk("--------------%s--------------\n",__FUNCTION__);
	return 0;
}
//模块卸载函数
static void __exit hello_drv_exit(void)
{
	printk("--------------%s--------------\n",__FUNCTION__);
}

//模块声明和认证
module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");
