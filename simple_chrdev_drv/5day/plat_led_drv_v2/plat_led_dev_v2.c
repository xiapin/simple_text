#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include "led_info.h"

#define LED_GPC0_CONF 0xE0200060
#define LED_GPC0_SIZE 8

#define BEEP_GPD0_CONF 0xE02000a0

struct led_platdata led_pdata = {
      .name = "gpc0_3/4",
      .shift = 3,
      .conf_reg_clear = 0xff,
      .conf_reg_data = 0x11,
      .data_reg = 0x3,
};
/*****************提供给drv的资源****************/
struct resource led_resource[] = {
    [0] = {
        .start = LED_GPC0_CONF,
        .end = LED_GPC0_CONF + LED_GPC0_SIZE -1,
        .name = "led_register",
        .flags = IORESOURCE_MEM,//类型
    },
    [1] = {
        .start = BEEP_GPD0_CONF,
        .end = BEEP_GPD0_CONF + LED_GPC0_SIZE -1,
        .name = "beep_register",
        .flags = IORESOURCE_MEM,
    },
    [2] = {
        .start = IRQ_EINT(0),
        .end = IRQ_EINT(0),
        .name = "up_key_register",
        .flags = IORESOURCE_IRQ,
    },
	
};

void led_dev_release(struct device *dev)
{
    
}

    
/******************pdev的定义*********************/   
struct platform_device led_pdev = {
    .name = "s5pv210_led",//匹配的名字
    .id = -1,
    .num_resources = ARRAY_SIZE(led_resource),
    .resource = led_resource,
    .dev = {
        .release = led_dev_release,
        .platform_data = &led_pdata,
    },
};

static int __init plat_led_dev_init(void)
{
    printk("--------------%s-------------\n",__FUNCTION__);
    /*注册一个pdev*/
    return platform_device_register(&led_pdev);
}
static void __exit plat_led_dev_exit(void)
{
    printk("--------------%s-------------\n",__FUNCTION__);
    /*注销一个pdev*/
    platform_device_unregister(&led_pdev);
}

module_init(plat_led_dev_init);
module_exit(plat_led_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LAOXIA@169.com");


