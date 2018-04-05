#include <linux/init.h>
#include <linux/module.h>

static int hello_init(void)
{
	printk(" Hello World enter!\n");
	return 0;
}

static int hello_exit(void)
{
	printk("Hello World exit!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Andy Gan");
MODULE_LICENSE("Dual BSD/GPL");