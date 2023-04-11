#include <linux/bsearch.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kthread.h>
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
#include <linux/kfifo.h>
#include <linux/delay.h>


#define DEVICE_NAME "hssi_driver"
#define DEVNODE		"hssi"

// Macros for TX and RX reset/enable
#define TRANSMIT_RESET		(1 << (0))
#define TRANSMIT_ENABLE		(1 << (1))
#define RECEIVE_RESET		(1 << (16))
#define RECEIVE_ENABLE		(1 << (17))

// TX and RX Working queue size
#define FIFO_SIZE 4096/4;


struct hssi_registers {
	u32 tx_data;	//W
	u32 rx_data;	//R
	u32 csr;
};


/* private data */
struct hssi_data {
	struct hssi_registers* __iomem regs;
	struct class* device_class;
	struct semaphore lock;
	struct cdev dev;
	struct task_struct* thread1, thread2;
	dev_t devnum;
	size_t offset;
	size_t read_count;
	int in_use;
	DECLARE_KFIFO(tx_fifo, u32, FIFO_SIZE);	// TX FIFO
	DECLARE_KFIFO(rx_fifo, u32, FIFO_SIZE);	// RX FIFO
};

static int initialized_devices;

void hssi_tx_thread(void* data) {
	struct hssi_data* priv = data;
	u32 data_to_send;
	
	while(true) {
		while(!kfifo_is_empty(&data->tx_fifo)) {
			kfifo_get(&data->tx_fifo, &data_to_send);
			while(!(data->regs->csr & (1 << 6))) { /* Probe the TX Ready bit until it is set */
				usleep_range(100, 200);	/* Yield CPU to other tasks for minimum 100us, maximum 200us */
			}
			/* When ready to transmit, store data_to_send in the TX reg and start transmission */
			data->regs->tx_data = data_to_send;
			data->regs->csr |= (1 << 2);
		}
		
	}
}

void hssi_rx_thread(void* data) {
	struct hssi_data* priv = data;
	u32 data_to_receive;
	
	while(true) {
		while (!(kfifo_is_full(&data->rx_fifo)) {
			kfifo_put(&data->rx_fifo, &data_to_receive);
			while(!(data->regs->csr & (1 << 18))) { /* Probe the RX_Data_Valid bit until it is set */
				usleep_range(100, 200);	/* Yield CPU to other tasks for minimum 100us, maximum 200us */
			}
			data->regs->rx_data = data_to_receive;
		}
	}
	
}

/* open */
static int
hssi_open(struct inode* i, struct file* f) {
	
	struct hssi_data* drv;
	int ret = 0;
	
	drv = container_of(i->i_cdev, struct hssi_data, dev);
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
hssi_read(struct file* f, char __user* buf, size_t bufsize, loff_t* pos) {
	
	struct hssi_data* drv = f->private_data;
	ssize_t ret = bufsize;
	ssize_t fifo_ret;
	INIT_KFIFO(drv->rx_fifo);

	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("read(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}

	fifo_ret = kfifo_to_user(&drv->rx_fifo, buf, bufsize, &ret);
	if (fifo_ret) {
		ret = fifo_ret;
		printk(KERN_INFO "Read failed");
		goto out;
	}
	
	printk(KERN_INFO "Read from RX FIFO");

}
out:
	up(&drv->lock);
	return ret;
}

/* release */
static int 
hssi_release(struct inode* i, struct file* f) {

	struct hssi_data* drv = f->private_data;
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
hssi_ioctl(struct file* f, unsigned int cmd, unsigned long arg) {

	struct hssi_data* drv = f->private_data;
	long int ret = -;
	
	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("ioctl(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}
	
	printk(KERN_INFO "ioctl()");
	
	switch() {
	/* TX reset */
		case _IO('A', 'r'):
			drv->regs->csr |= TRANSMIT_RESET;
			drv->regs->csr &= ~(TRANSMIT_ENABLE
								| TRANSMIT_RESET);
			break;
			
	/* TX disable */
		case _IO('A', 'd'):
			drv->regs->csr &= ~TRANSMIT_ENABLE;
			break;
			
	/* TX enable */
		case _IO('A', 'e'):
			drv->regs->csr |= TRANSMIT_ENABLE;
			break;
			
	/* RX reset */
		case _IO('A', 'c'):
			drv->regs->csr |= RECEIVE_RESET;
			drv->regs->csr &= ~(RECEIVE_ENABLE
								| RECEIVE_RESET);
			break;
		
	/* RX disable */ 
		case _IO('A', 'n'):
			drv->regs->csr &= ~RECEIVE_ENABLE;
			break;
	
	/* RX enable */
		case _IO('A', 'b'):
			drv->regs->csr |= RECEIVE_ENABLE;
			break;
		}

out:
	up(&drv->lock);
	return ret;
	
}


/* write */
static ssize_t
hssi_write(struct file* f, const char __user* buf, size_t bufsize, loff_t* pos) {
	
	struct hssi_data* drv = f->private_data;
	ssize_t ret = bufsize;
	ssize_t fifo_ret;
	
	INIT_KFIFO(drv->tx_fifo);
	
	printk(KERN_INFO "writing()");

	if(down_interruptible(&drv->lock)) {
		return -ERESTARTSYS;
	}

	if(!drv->in_use) {
		pr_err("write(): attempted to close a device not in use");
		ret = -ENODEV;
		goto out;
	}

	
	fifo_ret = kfifo_from_user(&drv->tx_fifo, buf, bufsize, &ret);
	if(fifo_ret) {
		ret = fifo_ret;
		printk(KERN_INFO "Write failed");
		goto out;
	}
	
	printk(KERN_INFO "wrote to TX FIFO");
	
out:
	up(&drv->lock);
	return ret;
}

/* file operations */
static const struct file_operations fops = {
		.owner = THIS_MODULE,
		.open = hssi_open,
		.read = hssi_read,
		.release = hssi_release,
		.unlocked_ioctl = hssi_ioctl,
		.write = hssi_write,
	};


/** the probe function */
int hssi_driver_probe(struct platform_device* pdev) {
	struct device* dev = &pdev->dev;
	struct resource* res;
	struct hssi_data* priv;
	dev_t dev_current;

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


	alloc_chrdev_region(&priv->devnum, 0, DEVICE_NAME);
	priv->device_class = class_create(THIS_MODULE, DEVICE_NAME);


	/* initialize semaphore */
	sema_init(&priv->lock, 1);

	priv->thread = kthread_run(hssi_tx_thread, dev, "hssi_tx_thread_%d",
								initialized_devices);
								
	priv->thread = kthread_run(hssi_rx_thread, dev, "hssi_rx_thread_%d",
								initialized_devices);

	/* initialize device node for this device */
	cdev_init(&priv->dev, &fops);
	dev_current = MKDEV(MAJOR(priv->devnum), MINOR(priv->devnum));

	device_create(priv->device_class, NULL, dev_current, priv, "hssi%d",
			initialized_devices);

	cdev_add(&priv->dev, dev_current, 1);

	/* increment initialized count */
	initialized_devices = 1;
	dev_info(dev, "successfully probed driver");

	return 0;
}

/* remove function */
int hssi_driver_remove(struct platform_device* pdev) {
	struct device* dev = &pdev->dev;
	struct hssi_data* priv;
	dev_t dev_current;

	priv = dev_get_drvdata(dev);
	
	/* stop the thread in case we need to */
	kthread_stop(priv->thread1);
	kthread_stop(priv->thread2);

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
static const struct of_device_id hssi_ids[] = {
		{ .compatible = "capstone,hssi_driver" },
		{ }
	};

MODULE_DEVICE_TABLE(of, hssi_ids);

/** platform driver declaration */
static struct platform_driver hssi_platform_driver = {
		.driver = {
			.owner = THIS_MODULE,
			.name = "hssi-driver",
			.of_match_table = hssi_ids
		},
		.probe = hssi_driver_probe,
		.remove = hssi_driver_remove
	};

module_platform_driver(hssi_platform_driver);

MODULE_DESCRIPTION("UML Capstone HSSI driver");
MODULE_AUTHOR("Aidan McGuinness");
MODULE_LICENSE("GPL v2");
#endif

