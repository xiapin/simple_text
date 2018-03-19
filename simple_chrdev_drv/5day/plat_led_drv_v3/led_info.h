#ifndef __LED_INFO_H__
#define __LED_INFO_H__

#define LED_GPC0_CONF 0xE0200060
#define LED_GPC0_SIZE 8

struct led_platdata{
      char *name;
      int shift;
      int conf_reg_data;
      int conf_reg_clear;
      int data_reg;
};

struct s5pv210_led{
      int dev_major;
      struct class *cls;
      struct device *dev;
      int value;
      void *reg_base;
      struct led_platdata *pdata;

};

#endif
