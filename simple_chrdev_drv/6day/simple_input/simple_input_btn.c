#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/timer.h>

/*定义一个设备包*/
struct key_type{
    /*定义一个input设备*/
    struct input_dev *btn_input;
    /*工作队列*/
    struct work_struct btn_work;
    /*定时器*/
    struct timer_list btn_timmer;
};

struct key_type *key_type;


/*定义一个按键信息包*/
struct buttons{
      char *name;
      unsigned int irqno;
      int gpio;
      int code;
      unsigned long flags;
};

struct buttons buttons_set[] = {
    [0] = {
        .name = "key_up",
        .irqno = IRQ_EINT(0),
        .gpio = S5PV210_GPH0(0),
        .code = KEY_UP,
        .flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
    [1] = {
        .name = "key_down",
        .irqno = IRQ_EINT(1),
        .gpio = S5PV210_GPH0(1),
        .code = KEY_DOWN,
        .flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
    [2] = {
        .name = "key_left",
        .irqno = IRQ_EINT(2),
        .gpio = S5PV210_GPH0(2),
        .code = KEY_LEFT,
        .flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
    [3] = {
        .name = "key_right",
        .irqno = IRQ_EINT(3),
        .gpio = S5PV210_GPH0(3),
        .code = KEY_RIGHT,
        .flags= IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
    },
};

/******************定时器处理函数*************************************/
void key_dithering_timer(unsigned long work)
{
    /*消抖：
    *①发生中断后，中断处理函数负责调度work_queue工作队列处理中断事件
    *②工作队列处理函数使用定时器，调用mod_timer，产生一个滴答数
    *③短暂延时后在定时器处理函数中提交input_event;
    */
    printk("--------------%s----------%d\n",__FUNCTION__,__LINE__);
}

/******************工作队列函数*************************************/
void work_button_event(struct work_struct *work)
{
    printk("--------------%s----------%d\n",__FUNCTION__,__LINE__);
    mod_timer(&key_type->btn_timmer, jiffies + (HZ/50));
}

/******************中断处理函数*************************************/
irqreturn_t button_interrupt(int irqno, void *dev)//dev_id为传过来的参数
{
    struct buttons *bt_set;
    int value;
    printk("--------------%s----------\n",__FUNCTION__);


    bt_set = (struct buttons*)dev;

    value = gpio_get_value(bt_set->gpio);
    if(value){/*上报数据,抬起状态*/
#if 0
        input_report_key(struct input_dev * dev, unsigned int code, int value)
#else
        input_event(key_type->btn_input, EV_KEY,bt_set->code,0);/*抬起状态，value=0*/
        /*同步数据*/
#endif
    }else{
        input_event(key_type->btn_input, EV_KEY,bt_set->code,1);

    }
	 /*同步数据*/
	input_sync(key_type->btn_input);
    /*允许工作队列调度线程*/
    schedule_work(&key_type->btn_work);
    
    return IRQ_HANDLED;
}

/**********************模块加载函数**********************************/
static int __init simple_btn_input_init(void)
{
    int index;
    int ret;
   
    printk("--------------%s----------\n",__FUNCTION__);

    /*0.实例化设备对象*/
    key_type = kzalloc(sizeof(struct key_type), GFP_KERNEL);
    if(key_type == NULL){
        printk("kzalloc failed\n");
        return -ENOMEM;
    }
    
    /*1.分配input device对象*/
    key_type->btn_input = input_allocate_device();
    if(key_type->btn_input == NULL)
    {
        printk("input_allocate_device error\n");
        return -ENOMEM;
    }
    /*2.初始化input device对象------------------------>>>>>>>>设置位表*/
    
    //该设备能够产生哪种数据类型---EV_KEY表示产生按键数据
    key_type->btn_input->name = "simple_btn";
    
   // key_type->btn_input->evbit[0] |= BIT_MASK(EV_KEY);
    key_type->btn_input->evbit[BIT_WORD(EV_KEY)] |= BIT_MASK(EV_KEY);
    //能够产生哪个按键---比如能够产生下键 KEY_DOWN, KEY_ESC
	// btn_input->keybit[108/32] |= 1<<(108%32);
#if 0
   
    btn_input->keybit[BIT_WORD(KEY_DOWN)] |= BIT_MASK(KEY_DOWN);
    //__set_bit(KEY_DOWN,btn_input->keybit);
#else
    /*使能上下左右*/
    for(index = 0;index < ARRAY_SIZE(buttons_set);index++)
        key_type->btn_input->keybit[BIT_WORD(buttons_set[index].code)] |= BIT_MASK(buttons_set[index].code);
        
#endif

    /*3.注册input devices对象*/
    ret = input_register_device(key_type->btn_input);
    if(ret < 0){
        printk("input_register_device error\n");
        goto err_free_input;
    }
    
    /*4.硬件初始化-->申请中断*/
 #if 0
    irqno = IRQ_EINT(1);
    ret = request_irq(irqno,button_interrupt,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, 
                    "KEY_DOWN",NULL);
    if(ret < 0){
        printk("request_irq error/n");
        ret = -EFAULT;
        goto err_unregister_device;
    }
#else
    for(index = 0;index < ARRAY_SIZE(buttons_set);index++)
    {
    ret = request_irq(buttons_set[index].irqno,button_interrupt,buttons_set[index].flags,
                    buttons_set[index].name,&buttons_set[index]);
    if(ret < 0)
    {                                   
        printk("%s request_irq error\n",buttons_set[index].name);
        ret = -EFAULT;
        goto err_unregister_device;
    }
    }
#endif
    /*6.工作队列初始化*/
    INIT_WORK(&key_type->btn_work,work_button_event);

    /*7.定时器初始化*/
    /*
    *①init
    *②指定处理函数
    *③向内核添加定时器
    */
    init_timer(&key_type->btn_timmer);
    key_type->btn_timmer.function = key_dithering_timer;
    add_timer(&key_type->btn_timmer);
    
    return 0;

err_unregister_device:
    input_unregister_device(key_type->btn_input);
err_free_input:
    input_free_device(key_type->btn_input);

    return ret;
}

static void __exit simple_btn_input_exit(void)
{
    int i;
    printk("--------------%s----------\n",__FUNCTION__);
    input_unregister_device(key_type->btn_input);
    input_free_device(key_type->btn_input);
   
    for(i = 0;i < ARRAY_SIZE(buttons_set);i++)
                                       /*与irq_request最后一个参数一致*/
        free_irq(buttons_set[i].irqno,&buttons_set[i]);
}


module_init(simple_btn_input_init);
module_exit(simple_btn_input_exit);
MODULE_LICENSE("GPL");

