i2c_driver编写步骤

1.i2c_add_driver(struct i2c_driver)-->实现 struct i2c_driver
									-->.id_table-->name:匹配static struct i2c_board_info 				  smdkv210_i2c_devs0[] __initdata = {
												{ I2C_BOARD_INFO("at24c02", 0x50), },     /* Samsung S524AD0XD1 */
											  { I2C_BOARD_INFO("wm8580", 0x1b), },
												};
												-->i2c_device_id-->driver_data:存放各种设备信息

2.实现struct i2c_driver probe方法
	-->创建misc设备
		-->实例化fop
			-->实现write/read
				-->实现i2c_send/i2c_recv