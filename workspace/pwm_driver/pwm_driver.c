#include <linux/bsearch.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/property.h>
#include <linux/semaphore.h>
#include <linux/sysfs.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>

#ifndef PWM_DRIVER_IOCTLS_H__
#define PWM_DRIVER_IOCTLS_H__

#define DEVICE_NAME "pwm_driver"
#define DEVNODE		"pwm_audio"
#define MAX_MINOR_NUMBERS 5

#define STATUS_CONTROL_RESET			(1 << (0))
#define STATUS_CONTROL_RESET_OFFSET		(0)
#define STATUS_CONTROL_RESET_MASK		(1 << (0))
#define STATUS_CONTROL_ENABLE			(1 << (1))
#define STATUS_CONTROL_ENABLE_OFFSET	(1)
#define STATUS_CONTROL_ENABLE_MASK		(1 << (1))


struct pwm_registers {
	u32 samples;
	u32 rate;
	u32 scale;
	u32 status_control;
};

/* private data */
struct pwm_data {
	struct pwm_registers* __iomem regs;
	struct class* device_class;
	struct semaphore lock;
	struct cdev dev;
	dev_t devnum;
	char* bram;
	size_t offset;
	size_t read_count;
	int in_use;
};


void bram_vma_open(struct vm_area_struct *vma)
{
	printk(KERN_NOTICE "BRAM VMA open, virt %lx, phys %lx\n",
			vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

void bram_vma_close(struct vm_area_struct *vma)
{
	printk(KERN_NOTICE "BRAM VMA close.\n");
}


static int initialized_devices;

/* open */
static int
bram_open(struct inode* i, struct file* f) {
	
	struct pwm_data* drv;
	int ret = 0;
	
	drv = container_of(i->i_cdev, struct pwm_data, dev);
	f->private_data = drv;
	
	if(down_interruptible(&drv->lock)) {
		/* acquiring the semaphore is interruptible */
		return -ERESTARTSYS;
	}

	if(drv->in_use) {
		/* someone else is using the device */
		ret = -EBUSY;
		goto out;
	}

	/* we are using the device */
	drv->in_use = 1;
	drv->offset = 0;
	printk(KERN_INFO "device open");
	
out:
	up(&drv->lock);
	return ret;
	printk(KERN_INFO "device busy");
}

/* read */
static ssize_t
bram_read(struct file* f, char __user* buf, size_t bufsize, loff_t* pos) {
	
	struct pwm_data* drv = f->private_data;
	ssize_t ret = bufsize;
	size_t bufoffset = 0;

	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("read(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}
	
	/*Assumption: BRAM size is 4KiB*/
	while(bufsize) {
		if(bufsize + drv->offset >= 4096) {
			if(copy_to_user(buf + bufoffset, drv->bram + drv->offset, 4096 - drv->offset)) {
				ret = -EFAULT;
				goto out;
			}
			bufsize -= 4096 - drv->offset;
			bufoffset += 4096 - drv->offset;
			drv->offset = 0;
			*pos = 0;
		} else {
			if(copy_to_user(buf + bufoffset, drv->bram + drv->offset, bufsize)) {
				ret = -EFAULT;
				goto out;
			}
			drv->offset += bufsize;
			*pos = drv->offset;
			bufsize = 0;
		}	
	}
	
out:
	up(&drv->lock);
	return ret;
}

/* release */
static int 
bram_release(struct inode* i, struct file* f) {

	struct pwm_data* drv = f->private_data;
	int ret;

	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("close(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}

	drv->in_use = 0;
	drv->read_count = 0;
	printk(KERN_INFO "device released");


out:
	up(&drv->lock);
	return ret;
}

/* ioctl */
static long int
bram_ioctl(struct file* f, unsigned int cmd, unsigned long arg) {

	struct pwm_data* drv = f->private_data;
	int ret = -EINVAL;

	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("ioctl(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}
	
	printk(KERN_INFO "ioctl()");
	
	switch(cmd) {
		
		
		/* status_control 
		case _IOW('A', 's', int):
			 set status register to arg 
			drv->regs->status_control = arg;
			break;
		case _IOR('A', 's', int*):
			 copy value of status register into userspace address
			 * pointed to by arg 
			if (copy_to_user((u32 __user*)arg, &drv->regs->status_control, 4)) {
				goto out;
			}
			break;
		*/
		
		/* reset */
		case _IO('A', 's'):	/* reset */
			drv->regs->status_control |= STATUS_CONTROL_RESET;
			drv->regs->status_control &= ~(STATUS_CONTROL_ENABLE
											| STATUS_CONTROL_RESET);
			break;
			
		case _IO('A', 'd'):	/* disable */
			drv->regs->status_control &= ~STATUS_CONTROL_ENABLE;
			break;
			
		case _IO('A', 'e'): /* enable */
			drv->regs->status_control |= (1 << 1);
			break;
			
		
		/* samples */
		case _IOW('A', 'p', int):
			drv->regs->samples = arg;
			break;
		case _IOR('A', 'p', int*):
			if (copy_to_user((u32 __user*)arg, &drv->regs->samples, 4)) {
				goto out;
			}
			break;
			
		/* rate */	
		case _IOW('A', 'r', int):
			drv->regs->rate = arg;
			break;
		case _IOR('A', 'r', int*):
			if (copy_to_user((u32 __user*)arg, &drv->regs->rate, 4)) {
				goto out;
			}
			break;
			
		/* scale */
		case _IOW('A', 'c', int):
			drv->regs->scale = arg;
			break;
		case _IOR('A', 'c', int*):
			if (copy_to_user((u32 __user*)arg, &drv->regs->scale, 4)) {
				goto out;
			}
			break;
			
		default:
			goto out;
		}
	
	ret = 0;
out:
	up(&drv->lock);
	return ret;	
}

/* llseek */
static loff_t bram_seek(struct file* f, loff_t offset, int whence) {

	struct pwm_data *drv = f->private_data;
	loff_t newpos = -EINVAL;
		
	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("llseek(): attempted to close a device not in use");
		newpos = -ENODEV;
		goto out;
	}

	switch(whence) {
	  case SEEK_SET: /* SEEK_SET */
		newpos = offset;
		break;

	  case SEEK_CUR: /* SEEK_CUR */
		newpos = f->f_pos + offset;
		break;

	  case SEEK_END: /* SEEK_END */
		newpos = drv->offset /* + off */;
		break;

	  default: /* can't happen */
		goto out;
	}
	
	if (newpos < 0) {
		goto out;
	}
	
	f->f_pos = newpos;
	drv->offset = newpos;
out:
	up(&drv->lock);
	return newpos;
}


/* write */
static ssize_t
bram_write(struct file* f, const char __user* buf, size_t bufsize, loff_t* pos) {
	
	struct pwm_data* drv = f->private_data;
	ssize_t ret = bufsize;
	unsigned long bufoffset = 0;
	
	printk(KERN_INFO "writing()");

	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("write(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}
	
	/*Assumption: BRAM size is 4KiB*/
	while(bufsize) {
		if(bufsize + drv->offset >= 4096) {
			if(copy_from_user(drv->bram + drv->offset, buf + bufoffset, 4096 - drv->offset)) {
				ret = -EFAULT;
				goto out;
			}
			bufsize -= 4096 - drv->offset;
			bufoffset += 4096 - drv->offset;
			drv->offset = 0;
			*pos = 0;
		} else {
			if(copy_from_user(drv->bram + drv->offset, buf + bufoffset, bufsize)) {
				ret = -EFAULT;
				goto out;
			}
			drv->offset += bufsize;
			drv->regs->samples = drv->offset;
			*pos = drv->offset;
			bufsize = 0;
		}	
	}
	
/*
	if(access_ok(buf, bufsize)) {
		overflow = copy_from_user(drv->bram + drv->offset, buf, bufsize);
		drv->offset += bufsize - overflow;
		drv->regs->samples = drv->offset;
		ret = bufsize - overflow;
	} else {
		ret = -EINVAL;
		goto out;
	}
*/
	printk(KERN_INFO "wrote to device()");

out:
	up(&drv->lock);
	return ret;
}

/* mmap */
static int 
bram_mmap(struct file* f, struct vm_area_struct* vma) {
	
	struct pwm_data* drv = f->private_data;
	int ret;

	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("mmap(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}
	
	printk(KERN_INFO "trying to mmap \nflags: %ld\nstart: %lx\nend: %lx\noffset: %#lx\npageprot: %x", 
			vma->vm_flags, 
			vma->vm_start,
			vma->vm_end,
			vma->vm_pgoff,
			vma->vm_page_prot);
		
	//if(f->f_flags & O_SYNC) {
		//vma->vm_flags |= VM_IO;
	//}
	
	vma->vm_page_prot = phys_mem_access_prot(f, 0x40008000 >> PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);

	/*
	printk(KERN_INFO "modified prot \nflags: %ld\nstart: %lx\nend: %lx\noffset: %#lx\npageprot: %x", 
			vma->vm_flags, 
			vma->vm_start,
			vma->vm_end,
			vma->vm_pgoff,
			vma->vm_page_prot);
	*/
	//vma->vm_flags |= VM_RESERVED;
	
	if (remap_pfn_range(vma, vma->vm_start, 0x40008000U >> PAGE_SHIFT,
				    vma->vm_end - vma->vm_start,
				    vma->vm_page_prot)) {
		return -EAGAIN;
	}
		
	//vma->vm_ops = &bram_remap_vm_ops;
	//bram_vma_open(vma);
	
out:
	up(&drv->lock);
	return 0;

}

/* file operations */
static const struct file_operations fops = {
		.owner = THIS_MODULE,
		.open = bram_open,
		.read = bram_read,
		.release = bram_release,
		.unlocked_ioctl = bram_ioctl,
		.write = bram_write,
		.mmap = bram_mmap,
		.llseek = bram_seek
	};


/** the probe function */
int pwm_driver_probe(struct platform_device* pdev) {
	struct device* dev = &pdev->dev;
	struct resource* res;
	struct pwm_data* priv;
	//u32 revision;
	dev_t dev_current;
	//int ret;

	if(initialized_devices == 1 /*max_devices*/) {
		/* we only support max_devices */
		return -ENODEV;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if(!priv) {
		dev_err(dev, "unable to allocate driver data\n");
		return -ENOMEM;
	}
	dev_set_drvdata(dev, priv);

	/* mm registers */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res) {
		dev_err(dev, "unable to get resource\n");
		return -ENXIO;
	}

	/* remap the registers into the kernel's virtual address space; the kernel
	 * also makes use of the MMU so we need to map physical addresses to
	 * virtual addresses even for memory mapped registers */
	priv->regs = devm_ioremap_resource(dev, res);
	if(IS_ERR(priv->regs)) {
		dev_err(dev, "unable to remap I/O memory\n");
		return PTR_ERR(priv->regs);
	}

	/* bram */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if(!res) { 
		dev_err(dev, "unable to get resource\n");
		return -ENXIO;
	}
	
	/* remap bram into kernel's virtual address space */
	priv->bram = devm_ioremap_resource(dev, res);
	if(IS_ERR(priv->bram)) {
		dev_err(dev, "unable to remap I/O memory");
		return PTR_ERR(priv->bram);
	}

	alloc_chrdev_region(&priv->devnum, 0, 1, DEVICE_NAME);
	priv->device_class = class_create(THIS_MODULE, DEVICE_NAME);


	/* initialize semaphore */
	sema_init(&priv->lock, 1);


	/* initialize device node for this device */
	cdev_init(&priv->dev, &fops);
	dev_current = MKDEV(MAJOR(priv->devnum), MINOR(priv->devnum));

	device_create(priv->device_class, NULL, dev_current, priv, "audioctl%d",
			initialized_devices);

	cdev_add(&priv->dev, dev_current, 1);

	/* increment initialized count */
	initialized_devices = 1;
	dev_info(dev, "successfully probed driver");
	dev_info(dev, "registers at %p", priv->regs);
	dev_info(dev, "bram at %p", priv->bram);
	//dev_info(dev, "registers physical address at %x", virt_to_phys(priv->regs));
	//dev_info(dev, "bram physical address at %x", virt_to_phys(priv->bram));
	return 0;
}

/* remove function */
int pwm_driver_remove(struct platform_device* pdev) {
	struct device* dev = &pdev->dev;
	struct pwm_data* priv;
	dev_t dev_current;

	priv = dev_get_drvdata(dev);

	/* remove all character devices */
	cdev_del(&priv->dev);
	dev_current = MKDEV(MAJOR(priv->devnum),
			MINOR(priv->devnum));
	device_destroy(priv->device_class, dev_current);
	
	class_destroy(priv->device_class);
	unregister_chrdev_region(priv->devnum, MINORMASK);

	return 0;
}

/** device compatibility list */
static const struct of_device_id pwm_ids[] = {
		{ .compatible = "capstone,pwm_driver" },
		{ }
	};

MODULE_DEVICE_TABLE(of, pwm_ids);

/** platform driver declaration */
static struct platform_driver pwm_platform_driver = {
		.driver = {
			.owner = THIS_MODULE,
			.name = "pwm-driver",
			.of_match_table = pwm_ids
		},
		.probe = pwm_driver_probe,
		.remove = pwm_driver_remove
	};

module_platform_driver(pwm_platform_driver);

MODULE_DESCRIPTION("UML Capstone PWM audio driver");
MODULE_AUTHOR("Aidan McGuinness");
MODULE_LICENSE("GPL v2");
#endif

