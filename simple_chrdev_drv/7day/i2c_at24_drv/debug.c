i2c_driver��д����

1.i2c_add_driver(struct i2c_driver)-->ʵ�� struct i2c_driver
									-->.id_table-->name:ƥ��static struct i2c_board_info 				  smdkv210_i2c_devs0[] __initdata = {
												{ I2C_BOARD_INFO("at24c02", 0x50), },     /* Samsung S524AD0XD1 */
											  { I2C_BOARD_INFO("wm8580", 0x1b), },
												};
												-->i2c_device_id-->driver_data:��Ÿ����豸��Ϣ

2.ʵ��struct i2c_driver probe����
	-->����misc�豸
		-->ʵ����fop
			-->ʵ��write/read
				-->ʵ��i2c_send/i2c_recv