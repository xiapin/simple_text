#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>


int my_bus_match(struct device *dev, struct device_driver *drv)
{
    int ret;
    printk("***********%s***********\n",__FUNCTION__);

    ret = strcmp(dev->kobj.name, drv->name);
    return 0;
}
struct bus_type my_bus = {
    .name = "mybus",

	.match = my_bus_match,
};

EXPORT_SYMBOL(my_bus);
    
int __init bus_init(void)
{
     printk("***********%s***********\n",__FUNCTION__);
    return bus_register(&my_bus);
}
void __exit bus_exit(void)
{
     printk("***********%s***********\n",__FUNCTION__);
    bus_unregister(&my_bus);
}



module_init(bus_init);
module_exit(bus_exit);
MODULE_LICENSE("GPL");








