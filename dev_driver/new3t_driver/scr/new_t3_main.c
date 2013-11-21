#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/poll.h>

#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */
#include "newt3.h"
#include <linux/sched.h>

//Variable used
int newt3_major = NEWT3_MAJOR;
dev_t newt3_devno;
/*
 * Our parameters which can be set at load time.
 */
char *whom = "newt3";
int newt3_driver_nr_devs = NEWT3_NR_DEVS;	/* number of bare newt3 devices */
int newt3_minor =   0;
int newt3_buffer = BUFFERSIZE;
//module_param(whom, charp, S_IRUGO);
//module_param(newt3_driver_nr_devs, int, S_IRUGO);
//module_param(newt3_minor, int, S_IRUGO);
//module_param(newt3_qset, int, S_IRUGO);
//module_param(newt3_quantum, int, S_IRUGO);

MODULE_AUTHOR("Nicola Bettin");
MODULE_LICENSE("Dual BSD/GPL");

struct newt3_dev *newt3_devices;	/* allocated in newt3_init_module */
static int newt3_getwritespace(struct newt3_dev *dev, struct file *filp);
static int spacefree(struct newt3_dev *dev);

/* How much space is free? */
static int spacefree(struct newt3_dev *dev)
{
	PDEBUG(" IN spacefree\n");
	if (dev->rp == dev->wp)
		return dev->buffersize - 1;
	return ((dev->rp + dev->buffersize - dev->wp) % dev->buffersize) - 1;
}

/* Wait for space for writing; caller must hold device semaphore.  On
 * error the semaphore will be released before returning. */
static int newt3_getwritespace(struct newt3_dev *dev, struct file *filp)
{
	PDEBUG(" IN newt3_getwritespace\n");
	while (spacefree(dev) == 0) { /* full */
		DEFINE_WAIT(wait);

		up(&dev->sem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" writing: going to sleep\n",current->comm);
		prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
		if (spacefree(dev) == 0)
			schedule();
		finish_wait(&dev->outq, &wait);
		if (signal_pending(current))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	return 0;
}

static int newt3_fasync(int fd, struct file *filp, int mode)
{
	struct newt3_dev *dev = filp->private_data;
	PDEBUG(" IN newt3_fasync : mode = %d \n",mode);
	return fasync_helper(fd, filp, mode, &dev->async_queue);
}

static unsigned int newt3_poll(struct file *filp, poll_table *wait)
{
	struct newt3_dev *dev = filp->private_data;
	unsigned int mask = 0;
	PDEBUG(" IN newt3_poll ");
	/*
	 * The buffer is circular; it is considered full
	 * if "wp" is right behind "rp" and empty if the
	 * two are equal.
	 */
	down(&dev->sem);
	poll_wait(filp, &dev->inq,  wait);
	PDEBUG(" newt3_poll::poll_wait(filp, &dev->inq,  wait);\n");
	poll_wait(filp, &dev->outq, wait);
	PDEBUG(" newt3_poll::poll_wait(filp, &dev->outq, wait);\n");
	if (dev->rp != dev->wp){
		mask |= POLLIN | POLLRDNORM;	/* readable */
		PDEBUG(" newt3_poll:: mask |= POLLIN | POLLRDNORM;\n");
	}
	if (spacefree(dev)){
		mask |= POLLOUT | POLLWRNORM;	/* writable */
		PDEBUG(" newt3_poll:: mask |= POLLOUT | POLLWRNORM;\n");
	}
	up(&dev->sem);
	return mask;
}

/*
 * The ioctl() implementation
 */

int newt3_ioctl(struct inode *inode, struct file *filp,unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int retval = 0;
	int count=0;
	struct newt3_dev *dev = filp->private_data;
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != NEWT3_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > NEWT3_IOC_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	switch(cmd) {

//	  case SCULL_IOCGQUANTUM: /* Get: arg is pointer to result */
//		retval = __put_user(scull_quantum, (int __user *)arg);
//		break;

	  case NEWT3_SIZEBUFF: /* Query: return it (it's positive) */
		return newt3_buffer;

	  case NEWT3_CHAR2REED:

			PDEBUG("IN IOCTL NEWT3_CHAR2REED\n");
			if (down_interruptible(&dev->sem))
				return -ERESTARTSYS;
			if (dev->rp == dev->wp)
				retval=0;
			else{
				if (dev->wp > dev->rp)
						count = min(count, (dev->wp - dev->rp));
				else /* the write pointer has wrapped, return data up to dev->end */
						count = min(count, (dev->end - dev->rp));
				retval=count;
			}
			up (&dev->sem);
			return retval;

	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}
	return retval;

}

/*
 * Data management: read and write
 */

static ssize_t newt3_read (struct file *filp, char __user *buf, size_t count,loff_t *f_pos)
{

	struct newt3_dev *dev = filp->private_data;
	PDEBUG("IN newt3_read\n");
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	while (dev->rp == dev->wp) { /* nothing to read */
		up(&dev->sem); /* release the lock */
		if (filp->f_flags & O_NONBLOCK){  //nel caso sia stato definito NON BLOCCANTE
			printk(KERN_NOTICE "newt3 :READ:: I'm no blocking!\n");
			return -EAGAIN;
		}
		PDEBUG("\"%s\" reading: going to sleep\n", current->comm);
		if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		/* otherwise loop, but first reacquire the lock */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	/* ok, data is there, return something */
	if (dev->wp > dev->rp)
		count = min(count, (size_t)(dev->wp - dev->rp));
	else /* the write pointer has wrapped, return data up to dev->end */
		count = min(count, (size_t)(dev->end - dev->rp));
	if (copy_to_user(buf, dev->rp, count)) {
		up (&dev->sem);
		return -EFAULT;
	}
	dev->rp += count;
	if (dev->rp == dev->end)
		dev->rp = dev->buffer; /* wrapped */
	up (&dev->sem);

	/* finally, awake any writers and return */
	wake_up_interruptible(&dev->outq);
	PDEBUG("\"%s\" did read %li bytes\n",current->comm, (long)count);
	return count;
}

static ssize_t newt3_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos){
	struct newt3_dev *dev = filp->private_data;
	int result;
	PDEBUG(" IN newt3_write\n");

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* Make sure there's space to write */
	result = newt3_getwritespace(dev, filp);
	if (result)
		return result; /* newt3_getwritespace called up(&dev->sem) */

	/* ok, space is there, accept something */
	count = min(count, (size_t)spacefree(dev));
	if (dev->wp >= dev->rp)
		count = min(count, (size_t)(dev->end - dev->wp)); /* to end-of-buf */
	else /* the write pointer has wrapped, fill up to rp-1 */
		count = min(count, (size_t)(dev->rp - dev->wp - 1));
	PDEBUG("Going to accept %li bytes to %p from %p\n", (long)count, dev->wp, buf);
	if (copy_from_user(dev->wp, buf, count)) {
		up (&dev->sem);
		return -EFAULT;
	}
	dev->wp += count;
	if (dev->wp == dev->end)
		dev->wp = dev->buffer; /* wrapped */
	up(&dev->sem);

	/* finally, awake any reader */
	wake_up_interruptible(&dev->inq);  /* blocked in read() and select() */

	/* and signal asynchronous readers, explained late in chapter 5 */
	if (dev->async_queue)
		kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
	PDEBUG("\"%s\" did write %li bytes\n",current->comm, (long)count);
	return count;

}

static int newt3_release(struct inode *inode, struct file *filp)
{
	struct newt3_dev *dev = filp->private_data;
	PDEBUG(" IN newt3_release\n");
	/* remove this filp from the asynchronously notified filp's */
	newt3_fasync(-1, filp, 0);
	down(&dev->sem);
	if (filp->f_mode & FMODE_READ)
		dev->nreaders--;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters--;
	if (dev->nreaders + dev->nwriters == 0) {
		kfree(dev->buffer);
		dev->buffer = NULL; /* the other fields are not checked on open */
	}
	up(&dev->sem);
	return 0;
}

static int newt3_open(struct inode *inode, struct file *filp)
{
	struct newt3_dev *dev;
	PDEBUG(" IN newt3_open");
	dev = container_of(inode->i_cdev, struct newt3_dev, cdev);
	filp->private_data = dev;

	if (down_interruptible(&dev->sem)) //decrementa il semaforo
		return -ERESTARTSYS;
	if (!dev->buffer) {
		/* allocate the buffer */
		dev->buffer = kmalloc(newt3_buffer, GFP_KERNEL);
		if (!dev->buffer) {
			up(&dev->sem);
			return -ENOMEM;
		}
	}
	dev->buffersize = newt3_buffer;
	dev->end = dev->buffer + dev->buffersize;
	dev->rp = dev->wp = dev->buffer; /* rd and wr from the beginning */

	/* use f_mode,not  f_flags: it's cleaner (fs/open.c tells why) */
	if (filp->f_mode & FMODE_READ)
		dev->nreaders++;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters++;
	filp->f_flags = O_NONBLOCK;
	up(&dev->sem);
	return nonseekable_open(inode, filp);
}

struct file_operations newt3_fops = {
	.owner =    THIS_MODULE,
	.llseek =	no_llseek,
	.read =     newt3_read,
	.write =    newt3_write,
	 .poll =    newt3_poll,
	.fasync =	newt3_fasync,
	.open =     newt3_open,
	.release =  newt3_release,
};

void newt3_cleanup_module(void){

	int i;
	dev_t devno = MKDEV(newt3_major, newt3_minor);
	PDEBUG(" IN newt3_cleanup_module");
	/* Get rid of our char dev entries */
	if (newt3_devices) {
			for (i = 0; i < newt3_driver_nr_devs; i++) {
				cdev_del(&newt3_devices[i].cdev);
				kfree(newt3_devices[i].buffer);
			}
			kfree(newt3_devices);
		}
		/* cleanup_module is never called if registering failed */
		unregister_chrdev_region(devno, newt3_driver_nr_devs);
		return;
}

static void newt3_setup_cdev(struct newt3_dev *dev, int index)
{
	int err, devno = MKDEV(newt3_major, newt3_minor + index);

	cdev_init(&dev->cdev, &newt3_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &newt3_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding newt3%d", err, index);
}

int newt3_init_module(void)
{
	int result, i;
	dev_t dev = 0;
	result = alloc_chrdev_region(&dev, newt3_minor, newt3_driver_nr_devs,whom);
		newt3_major = MAJOR(dev);
	if(result < 0){
		printk(KERN_WARNING "new3t: can't get major %d\n", newt3_major);
		return result;
	}
	PDEBUG(" Major = %d\n", newt3_major);
     /*
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	newt3_devices = kmalloc(newt3_driver_nr_devs * sizeof(struct newt3_dev), GFP_KERNEL);
	if (!newt3_devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(newt3_devices, 0, newt3_driver_nr_devs * sizeof(struct newt3_dev));
    /* Initialize each device. */
	for (i = 0; i < newt3_driver_nr_devs; i++) {
		init_waitqueue_head(&(newt3_devices[i].inq));
		init_waitqueue_head(&(newt3_devices[i].outq));
		init_MUTEX(&newt3_devices[i].sem);
		newt3_setup_cdev(&newt3_devices[i], i);
	}
    /* At this point call the init function for any friend device */
//	dev = MKDEV(newt3_major, newt3_minor + newt3_driver_nr_devs);
	return 0; /* succeed */

fail:
  	newt3_cleanup_module();
	return result;
}

module_init(newt3_init_module);
module_exit(newt3_cleanup_module);
