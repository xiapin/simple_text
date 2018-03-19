#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

extern struct bus_type my_bus;

int my_drv_probe(struct device *dev)
{
    printk("***********%s***********\n",__FUNCTION__);
    return 0;
}
int my_drv_remove(struct device *dev)
{
    printk("***********%s***********\n",__FUNCTION__);
    return 0;
}

struct device_driver my_bus_drv = {
        .name = "my_drv",
        .bus = &my_bus,
        .owner = THIS_MODULE,
        .probe = my_drv_probe,
        .remove = my_drv_remove,
};

int __init my_bus_drv_init(void)
{
    printk("***********%s***********\n",__FUNCTION__);
    return driver_register(&my_bus_drv);
}
void __exit my_bus_drv_exit(void)
{
     printk("***********%s***********\n",__FUNCTION__);
    driver_unregister(&my_bus_drv);
}



module_init(my_bus_drv_init);
module_exit(my_bus_drv_exit);
MODULE_LICENSE("GPL");








