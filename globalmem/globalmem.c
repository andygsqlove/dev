#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/coda.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>



#define MEM_SIZE 0X100
#define MEM_CLEAR 1
#define MEM_MAJOR 254
#define DEV_NAME "globalmem"

struct globalmem_dev
{
	struct cdev cdev;
	unsigned char mem[MEM_SIZE];
};

static struct globalmem_dev *dev;
static int major = 0;//MEM_MAJOR;
//static struct cdev;

static int globalmem_open(struct inode * inodep,struct file * filp)
{
	filp->private_data = dev;
	return 0;
}

static int globalmem_release(struct inode * inodep,struct file *filp)
{
	return 0;
}

static ssize_t globalmem_read (struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;
	int ret = 0;
	
	struct globalmem_dev *devp  = filp->private_data;
	if(p >= MEM_SIZE)
		return 0;
	if(count >= MEM_SIZE - p)
		count = MEM_SIZE - p;

	if(copy_to_user(buf, (void *)(devp ->mem + p), count))
	{
		return -EFAULT;
	}
	else
	{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "read %lu bytes(s) from %lu \n",(unsigned long)count,p);
	}

	return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t count, loff_t * ppos)
{
	unsigned long p = *ppos;
	int ret = 0;
	struct globalmem_dev * devp = filp->private_data;
	if(p >= MEM_SIZE)
		return 0;
	if(count >= MEM_SIZE - p)
		count = MEM_SIZE - p;

	if(copy_from_user((void *)(devp ->mem + p), buf, count))
	{
		return -EFAULT;
	}
	else
	{
		*ppos += count;
		ret = count;
		printk(KERN_INFO "write %lu bytes(s) from %lu\n",(unsigned long)count,p);
	}

	return ret;
}

static loff_t globalmem_llseek(struct file * filp, loff_t offset, int orig)
{
	loff_t ret = 0;
	switch(orig)
	{
		case 0: //从文件头开始
			if(offset < 0)
			{
				ret = -EINVAL;
				break;
			}
			//if((unsigned int)offset >= MEM_SIZE)
			//{
			//	ret = -EINVAL;
			//	break;
			//}
			
			filp->f_pos = (unsigned)offset;
			ret = filp->f_pos;
			break;

		case 1: //从当前位置开始
			if(offset < 0)
			{
				ret = -EINVAL;
				break;
			}
			if((offset + filp->f_pos) >= MEM_SIZE )
			{
				ret = -EINVAL;
				break;
			}

			filp->f_pos += offset;
			ret = filp->f_pos;
			break;
		default:
			ret = -EINVAL;

	}

	return ret;
}


static int globalmem_ioctl(struct inode * inodep, struct file * filp, unsigned int cmd, unsigned long arg)
{
	struct globalmem_dev *devp = filp->private_data;
	switch(cmd)
	{
		case MEM_CLEAR:
			memset(devp->mem,0,MEM_SIZE);
			break;
		default:
			return -EINVAL;
	}
	
	return 0;
}

static const struct file_operations memopt = {
	.owner=THIS_MODULE,
	.read=globalmem_read,
	.write=globalmem_write,
	.ioctl=globalmem_ioctl,
	.open=globalmem_open,
	.release=globalmem_release,
};


static void memchr_getup(struct globalmem_dev *dev)
{
	int err;
	dev_t devno = MKDEV(major, 0);
	cdev_init(&dev->cdev, &memopt);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &memopt;
	err = cdev_add(&dev->cdev, devno, 1);
	if(err)
		printk("cdev_add error!\n");
}

static int __init globalmem_init(void)
{
	int err;
	//申请设备号
	dev_t devno = MKDEV(major, 0);
	if(devno)
	{
		err = register_chrdev_region(devno, 1, DEV_NAME);
	}
	else
	{
		err = alloc_chrdev_region(&devno, 0, 1, DEV_NAME);
		major = MAJOR(devno);
	}

	if(err < 0)
		return -1;
	dev = kmalloc(sizeof(struct globalmem_dev),GFP_KERNEL);
	printk("addr = %lu\n",dev);
	if(!dev)
	{
		err = -EINVAL;
		goto fail_malloc;
	}
	memchr_getup(dev);

	return 0;
	
	fail_malloc: unregister_chrdev_region(devno,1);
	return err;
}

static void __exit globalmem_exit(void)
{
	cdev_del(&dev->cdev);
	kfree(dev);
	unregister_chrdev_region(MKDEV(major,0), 1);
}


module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_AUTHOR("andy");
MODULE_LICENSE("GPL");


