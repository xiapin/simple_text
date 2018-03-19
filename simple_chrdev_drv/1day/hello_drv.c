#include <linux/init.h>
#include <linux/module.h>


//module init
static int __init hello_drv_init(void)
{
      printk("-----------%s-------------\n",__FUNCTION__);
      return 0;
}
//module exit
static void __exit hello_drv_exit(void)
{
      printk("-----------%s-------------\n",__FUNCTION__);
}
//module declare and license
module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");

