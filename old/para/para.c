#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static char *book_name = "I love you";
static int num = 4000;

static int __init book_init(void)
{
	printk(" BOOK name is : %s\n",book_name);
	printk(" BOOK num is : %d\n",num);
	return 0;
}

static void __exit book_exit(void)
{
	printk("BOOK exit\n");
}

module_init(book_init);
module_exit(book_exit);
module_param(num,int,S_IRUGO);
module_param(book_name,charp,S_IRUGO);