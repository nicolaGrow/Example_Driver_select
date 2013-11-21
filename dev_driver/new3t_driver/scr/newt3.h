
#ifndef _NEWT3_H_
#define _NEWT3_H_


#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef NEWT3_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "NEWT3: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef NEWT3_MAJOR
#define NEWT3_MAJOR 0   /* dynamic major by default */
#endif

#ifndef NEWT3_NR_DEVS
#define NEWT3_NR_DEVS 4    /* newt3_driver_0 through newt3_driver_3 */
#endif

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define NEWT3_IOC_MAGIC  'g'
/* Please use a different 8-bit number in your code */

#define NEWT3_IOCRESET    _IO(NEWT3_IOC_MAGIC, 0)

#define NEWT3_CHAR2REED _IOR(NEWT3_IOC_MAGIC,  1, int)
#define NEWT3_SIZEBUFF  _IOR(NEWT3_IOC_MAGIC,2, int)
#define NEWT3_IOC_MAXNR 2

#ifndef BUFFERSIZE
#define BUFFERSIZE 1024*4
#endif

/*
 * Representation of scull quantum sets.
 */
struct newt3_qset {
	void **data;
	struct newt3_qset *next;
};

struct newt3_dev {
	struct semaphore sem;     /* mutual exclusion semaphore     */
	char *rp, *wp;                     /* where to read, where to write */
	int nreaders, nwriters;            /* number of openings for r/w */
	wait_queue_head_t inq, outq;       /* read and write queues */
	char *buffer, *end;                /* begin of buf, end of buf */
	int buffersize;                    /* used in pointer arithmetic */
	struct fasync_struct *async_queue; /* asynchronous readers */
	struct cdev cdev;	  /* Char device structure		*/

};


/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)		/* low  nibble */

/*
 * The different configurable parameters
 */
extern int newt3_driver_nr_devs;     /* newt3_driver.c */
extern char *whom;
extern int newt3_minor;
extern int newt3_quantum;
extern int newt3_qset;


void newt3_cleanup_module(void);
int newt3_init_module(void);
static int newt3_open(struct inode *inode, struct file *filp);
static int newt3_release(struct inode *inode, struct file *filp);
static ssize_t newt3_read (struct file *filp, char __user *buf, size_t count,  loff_t *f_pos);
static ssize_t newt3_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
static int newt3_fasync(int fd, struct file *filp, int mode);
static unsigned int newt3_poll(struct file *filp, poll_table *wait);
/*
 * Prototypes for shared functions
 */
#endif
