#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
//��ͬ�豸�����ݰ�
struct e2prom_private{
    char *name;//����
    int size;//������С
    int version;//�汾
};
/*����һ��ȫ�ֱ��������client�����Ϣ*/

struct i2c_e2prom_global{
    struct i2c_client *client;/*��ʼ����ֵ*/
    struct e2prom_private *private;
    
    struct miscdevice i2c_misc;
};

struct i2c_e2prom_global *at24_dev;

struct e2prom_private c02_private = {
     .name = "at240c02 in test",
     .size = 256,
     .version = 0x1234,
};
struct e2prom_private c04_private = {
     .name = "at240c04 in test",
     .size = 512,
     .version = 0x1234,
};
struct e2prom_private c08_private = {
     .name = "at240c08 in test",
     .size = 1024,
     .version = 0x1234,
};

/*��дi2c_master_recv ����*/
int at24_i2c_send(struct i2c_client *client,char *buf,int count)
{
    int ret;
    struct i2c_adapter *adapter = client->adapter;

    struct i2c_msg msg;
    msg.addr = client->addr;/*���豸��ַ��ƥ��֮��õ�*/
	msg.flags = 0;/*�͵�ƽ����*/
	msg.len = count;
	msg.buf = buf;
    /*����1��client��������
    * ����2����Ϣ��
    * ����3����Ϣ��ĸ���
    */
    ret = i2c_transfer(adapter, &msg,1);//���ͳɹ������ط��͵Ĵ���

    return ret==1?count:ret;
}

int at_24_i2c_recv(struct i2c_client *client,char *buf,int count)
{
    int ret;
    struct i2c_adapter *adapter = client->adapter;

    struct i2c_msg msg;
    msg.addr = client->addr;/*���豸��ַ��ƥ��֮��õ�*/
	msg.flags = 1;/*�ߵ�ƽ����*/
	msg.len = count;
	msg.buf = buf;
    /*����1��client��������
    * ����2����Ϣ��
    * ����3����Ϣ��ĸ���
    */
    ret = i2c_transfer(adapter, &msg,1);//���ͳɹ������ط��͵Ĵ���

    return ret==1?count:ret;
}
int at24_i2c_e2prom_open(struct inode *inode, struct file *flip)
{
    printk("----------%s-----------\n",__FUNCTION__);
    return 0;
}
ssize_t at24_i2c_e2prom_read(struct file *flip, char __user *buf, size_t count, loff_t *fpos)
{
    int ret;
    printk("----------%s-----------\n",__FUNCTION__);
    if(count < 0 || count > at24_dev->private->size)
        return -EINVAL;
    /*����һ��������*/
    char *temp = kzalloc(count,GFP_KERNEL);
    if(temp == NULL)
        return -ENOMEM;
    /*�ٴ�Ӳ����ȡ����*/
    //i2c_master_recv(const struct i2c_client * client, char * buf, int count)
    ret = at_24_i2c_recv(at24_dev->client,temp,count);
    if(ret < 0){
        printk("at_24_i2c_recv error\n");
        goto err_free;
    }
    /*���ϴ����û��ռ�*/
    ret = copy_to_user(buf,temp,count);
    if(ret > 0 ){
        printk("copy_to_user error\n");
        ret = -EFAULT;
        goto err_free;
    }

    kfree(temp);
    return count;
    
err_free:
    kfree(temp);
    return ret;
}

ssize_t at24_i2c_e2prom_write(struct file *flip, const char __user *buf, size_t count, loff_t *fops)
{
    int ret;
    printk("----------%s-----------\n",__FUNCTION__);
    if(count < 0 || count > at24_dev->private->size)
        return -EINVAL;
    /*����һ��������*/
    char *temp = kzalloc(count,GFP_KERNEL);
    if(temp == NULL)
        return -ENOMEM;
    /*�ٴ�Ӧ�û�ȡ����*/
    ret = copy_from_user(temp,buf,count);
    if(ret > 0){
        printk("copy_from_user failed\n");
        ret = -EFAULT;
        goto err_free;
    }
    
    /*��д��Ӳ��*/
    ret = at24_i2c_send(at24_dev->client,temp,count);
    if(ret < 0){
        printk("at24_i2c_send failed\n");
        goto err_free;
    }

    kfree(temp);
    return count;
    
err_free:
    kfree(temp);
    return ret;
}

int at24_i2c_e2prom_close(struct inode *inode, struct file *flip)
{
    printk("----------%s-----------\n",__FUNCTION__);
    return 0;
}

/********************��������******************/
struct file_operations at24_i2c_fops = {
    .open = at24_i2c_e2prom_open,
    .read = at24_i2c_e2prom_read,
    .write = at24_i2c_e2prom_write,
    .release = at24_i2c_e2prom_close,
};
                                        /*�˴���idΪһ����ַ���������豸�ĸ�����Ϣ*/
int at24_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{   
    /*��ʵ����ȫ�ֱ���*/
    at24_dev = kzalloc(sizeof(struct i2c_e2prom_global),GFP_KERNEL);
    if(at24_dev == NULL){
        printk("kzalloc failed\n");
        return -ENOMEM;
    }

    /*�ڼ�¼ƥ����i2c_client��Ϣ*/
    
    at24_dev->client = client;
    at24_dev->private = (struct e2prom_private*)id->driver_data;
    printk("client name = %s\n",client->name);
    printk("driver name = %s\n",at24_dev->private->name);
    printk("driver size = %d\n",at24_dev->private->size);
    printk("driver version = 0x%x\n",at24_dev->private->version);

    at24_dev->i2c_misc.name = "i2c_e2prom";/*�豸�ڵ������*/
    at24_dev->i2c_misc.fops = &at24_i2c_fops,/*��������*/
    at24_dev->i2c_misc.minor = MISC_DYNAMIC_MINOR,/*���豸�ţ�һ���Զ�����*/

    misc_register(&at24_dev->i2c_misc);
    
    return 0;
}
                                        
int at24_i2c_remove(struct i2c_client *client)
{
    printk("----------%s-----------\n",__FUNCTION__);
    misc_deregister(&at24_dev->i2c_misc);
    kfree(at24_dev);
    return 0;
}

/*����ƥ���id_table*/
struct i2c_device_id  at24_id_table[] = {
        {"at24c02",(unsigned long)&c02_private},
        {"at24c04",(unsigned long)&c04_private},
        {"at24c08",(unsigned long)&c08_private},
};

/*����driver����*/
struct i2c_driver at24_i2c_drv = {
    .probe = at24_i2c_probe,
    .remove = at24_i2c_remove,
    .driver = {
            .name = "at24_drv",//�����ļ���/sys/bus//i2c/drivers/at24_drv
            
    },
    .id_table = at24_id_table,
};



static int __init at24_drv_init(void)
{
    printk("----------%s-----------\n",__FUNCTION__);
    return i2c_add_driver(&at24_i2c_drv);
}

static void __exit at24_drv_exit(void)
{
    printk("----------%s-----------\n",__FUNCTION__);
    i2c_del_driver(&at24_i2c_drv);
}



module_init(at24_drv_init);
module_exit(at24_drv_exit);
MODULE_LICENSE("GPL");





