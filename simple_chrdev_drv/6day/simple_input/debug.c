
匹配过程
	struct input_dev {
		struct device dev;
		struct list_head	h_list;
		struct list_head	node;
	}
	
input_allocate_device()
---->evbit/keybit
	---->input_register_device
		---->list_add_tail(&dev->node, &input_dev_list);&&list_for_each_entry(handler, &											input_handler_list, node)
			<将当前input_dev中的node加入链表，并且遍历handler链表>
