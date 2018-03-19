#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

extern struct bus_type my_bus;

struct device my_bus_dev = {
    .init_name = "my_dev",
    .bus = &my_bus,
};

int __init my_bus_dev_init(void)
{
   printk("***********%s***********\n",__FUNCTION__);
   return device_register(&my_bus_dev);
}
void __exit my_bus_dev_exit(void)
{
    printk("***********%s***********\n",__FUNCTION__);
    device_unregister(&my_bus_dev);
}



module_init(my_bus_dev_init);
module_exit(my_bus_dev_exit);
MODULE_LICENSE("GPL");








