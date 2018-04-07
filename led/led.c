#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define LED_ADDR_BASE 0xe0200280
#define LED_MAJOR 250
#define DEV_NAME "LED"
#define LED_NUM  4
#define ON		1
#define OFF		0
struct led_dev
{
	struct cdev cdev;
	unsigned int states;
};

static struct led_dev *led_devp;
static unsigned int led_major = LED_MAJOR;
static unsigned long *led_gpio_base = NULL;
static unsigned long *led_gpio_con = NULL;
static unsigned char *led_gpio_data = NULL;
static struct resource *led_res = NULL;


static int  _led_ioctl(unsigned int cmd,unsigned long index)
{
	int ret;
	if(ON == index)
	{
		*led_gpio_data = *led_gpio_data & (~(0x1 << cmd));
		ret = ON;
	}
	else if(OFF == index)
	{
		*led_gpio_data = *led_gpio_data | (0x1 << cmd);
		ret = OFF;
	}
	else
	{
		printk(KERN_INFO "index error!\n");
		ret = -1;
	}
	return ret;
}
static int leddev_open (struct inode *inodep, struct file *filp)
{
	filp->private_data = led_devp;
	*led_gpio_con = 0x1111;//output
	*led_gpio_data = 0x0; //on
	printk(KERN_INFO "LED dev open!\n");
	return 0;
}
static int leddev_release (struct inode *inodep, struct file *filp)
{
	*led_gpio_data = 0xf;
	printk(KERN_INFO "LED dev release!\n");
	return 0;
}
static int leddev_ioctl (struct inode *inodep, struct file *filp, unsigned int cmd, unsigned long index)
{
	int result;
	struct led_dev *devp = filp->private_data;
	switch(cmd)
	{
		case 0:
				devp[0].states = _led_ioctl(0,index);
				result = 0;
				break;
		case 1:
				devp[1].states = _led_ioctl(1,index);
				result = 1;
				break;
		case 2:
				devp[2].states = _led_ioctl(2,index);
				result = 2;
				break;
		case 3:
				devp[3].states = _led_ioctl(3,index);
				result = 3;
				break;
		default:
			printk(KERN_INFO "led cmd error!\n");
			result = -1;
	}
	return result;
}

const static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = leddev_open,
	.release = leddev_release,
	.ioctl = leddev_ioctl,
};

static int led_getup_cdev(struct led_dev *led_devp,unsigned int index)
{
	int err = 0,devno = MKDEV(led_major, index);
	cdev_init(&led_devp->cdev, &led_fops);

	led_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&led_devp->cdev, devno, 1);
	if(err)
		printk(KERN_INFO "Error adding led cdev!\n");
	return err;
}

static int __init led_init(void)
{
	int result;
	dev_t devno = MKDEV(led_major,0);

	if(devno)
	{
		result = register_chrdev_region(devno, LED_NUM , DEV_NAME);
	}
	else
	{
		result = alloc_chrdev_region(&devno, 0,LED_NUM , DEV_NAME);
		led_major = MAJOR(devno);
	}

	if(result < 0)
		return result;

	led_devp = kmalloc(sizeof(struct led_dev) * LED_NUM,GFP_KERNEL);
	if(!led_devp)
	{
		result = -ENOMEM;
		goto fail_malloc;
	}

	led_getup_cdev(&(led_devp[0]),0);
	led_getup_cdev(&(led_devp[1]),1);
	led_getup_cdev(&(led_devp[2]),2);
	led_getup_cdev(&(led_devp[3]),3);

	led_res = request_mem_region(LED_ADDR_BASE, 8, "led_res");
	if(!led_res)
	{
		printk(KERN_INFO "request_mem_register failed!\n");
		goto failed_request_mem_region;
	}

	led_gpio_base = ioremap(LED_ADDR_BASE, 8);
	if(!led_gpio_base)
	{
		printk(KERN_INFO "ioremap failed!\n");
		goto failed_ioremap;
	}

	led_gpio_con = led_gpio_base + 0;
	led_gpio_data = led_gpio_base + 1;
	printk("led dev init successed !\n");
	failed_ioremap:
		release_mem_region(LED_ADDR_BASE,8);
	failed_request_mem_region:
			cdev_del(&(led_devp[0].cdev));
			cdev_del(&(led_devp[1].cdev));
			cdev_del(&(led_devp[2].cdev));
			cdev_del(&(led_devp[3].cdev));
	fail_malloc: 
		unregister_chrdev_region(devno, LED_NUM);
		return result;
}

static void __exit led_exit(void)
{
	iounmap(led_gpio_base);
	release_mem_region(LED_ADDR_BASE,8);
	cdev_del(&(led_devp[0].cdev));
	cdev_del(&(led_devp[1].cdev));
	cdev_del(&(led_devp[2].cdev));
	cdev_del(&(led_devp[3].cdev));
	unregister_chrdev_region(MKDEV(led_major,0), LED_NUM);
	kfree(led_devp);
}

module_init(led_init);
module_exit(led_exit);
module_param(led_major,int,S_IRUGO);

MODULE_AUTHOR("ANDY");
MODULE_LICENSE("GPL");



