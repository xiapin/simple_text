#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int baudrate = 9600;
static int port[]={0,1,2,3};
static char *name = "test";

module_param(baudrate,int,S_IRUGO);
module_param_array(port,int,NULL,S_IRUGO);
module_param(name,charp,S_IRUGO);

static int  test_drv_init(void)
{
      int i;

      printk("-----------%s-------------\n",__FUNCTION__);
      printk("baudrate:%d\n",baudrate);
      printk("port:");
      for(i = 0;i < ARRAY_SIZE(port);i++)
	    printk(" %d",port[i]);
      
      printk("\n");
      printk("name:%s\n",name);

      return 0;
}

static void  test_drv_exit(void)
{
      printk("***********%s*************\n",__FUNCTION__);
}

module_init(test_drv_init);
module_exit(test_drv_exit);
MODULE_LICENSE("GPL");




