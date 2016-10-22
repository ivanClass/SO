#include <linux/kernel.h>
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>       /*For fg_console*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kd.h>        /*For KDSETLED*/
#include <asm/uaccess.h>	 /*for copy_to_user*/
#include <linux/cdev.h>
#include <linux/vt_kern.h>

#define ALL_LEDS_ON 0x7
#define ALL_LEDS_OFF 0
#define LONGMAXCOMANDO 20
#define SUCCESS 0
#define DEVICE_NAME "leds"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static inline int set_leds(struct tty_driver* handler, unsigned int mask);
struct tty_driver* get_kbd_driver_handler(void);
static int modleds_init(void);
static void modleds_exit(void);



/*
 * Global variables are declared as static, so are global within the file.
 */

dev_t start;
struct cdev* chardev=NULL;
struct tty_driver* kbd_driver= NULL;
static int Device_Open = 0;	/* Is device open?
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;		/* This will be initialized every time the
				   device is opened successfully */
static int counter=0;		/* Tracks the number of times the character
				 * device has been opened */

MODULE_LICENSE("GPL");

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{

	int major;		/* Major number assigned to our device driver */
	int minor;		/* Minor number assigned to the associated character device */

	/* Get available (major,minor) range */
	if (alloc_chrdev_region (&start, 0, 1,DEVICE_NAME)){
		printk(KERN_INFO "Can't allocate chrdev_region()");
		return -ENOMEM;
	}

    /* Create associated cdev */
    if ((chardev=cdev_alloc())==NULL){
		printk(KERN_INFO "cdev_alloc() failed ");
		return -ENOMEM;
    }

    cdev_init(chardev,&fops);

    if (cdev_add(chardev,start,1)){
		printk(KERN_INFO "cdev_add() failed ");
		return -ENOMEM;
    }

    major=MAJOR(start);
    minor=MINOR(start);

    modleds_init();

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'sudo mknod -m 666 /dev/%s c %d %d'.\n", DEVICE_NAME, major,minor);
	printk(KERN_INFO "Try to cat and echo to the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	/* Destroy chardev */
	if (chardev){
		cdev_del(chardev);
		modleds_exit();
	}
	/*
	 * Unregister the device
	 */
	unregister_chrdev_region(start, 1);
}

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev"
 */
static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
		return -EBUSY;

	Device_Open++;

	/* Initialize msg */
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);

	/* Initially, this points to the beginning of the message */
	msg_Ptr = msg;

	/* Increase the module's reference counter */
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/*
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module.
	 */
	//modleds_exit();
	module_put(THIS_MODULE);

	return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer
	 */
	int bytes_to_read = length;

	/*
	 * If we're at the end of the message,
	 * return 0 -> end of file
	 */
	if (*msg_Ptr == 0)
		return 0;

	/* Make sure we don't read more chars than
	 * those remaining to read
    	 */
	if (bytes_to_read > strlen(msg_Ptr))
		bytes_to_read=strlen(msg_Ptr);

	/*
	 * Actually transfer the data onto the userspace buffer.
	 * For this task we use copy_to_user() due to security issues
	 */
	if (copy_to_user(buffer,msg_Ptr,bytes_to_read))
		return -EFAULT;

	/* Update the pointer for the next read operation */
	msg_Ptr+=bytes_to_read;

	/*
	 * The read operation returns the actual number of bytes
	 * we copied  in the user's buffer
	 */
	return bytes_to_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/chardev
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    int ledsEncenderApagar = 0;
    int led1 = 0x2; //ENCENDER LED NUMLOCK
    int led2 = 0x4;  //ENCENDER LED CAPSLOCK
    int led3 = 0x1;   //ENCENDER SCROLLLOCK
    int ledsOff = 0x0;
    char copiaBuff[LONGMAXCOMANDO+1];
   
    int error = 0;

   if (len > LONGMAXCOMANDO)
   	return -ENOMEM;

    if (copy_from_user(copiaBuff, buff, len))
            return -EFAULT;

    copiaBuff[len] = '\0';

    int i=0;

    for (i=0;i<len;i++){
        switch(copiaBuff[i]){
	    case '0': ledsEncenderApagar |= ledsOff;
		break;
            case '1': ledsEncenderApagar |= led1;
		break;
            case '2': ledsEncenderApagar |= led2;
		break;
            case '3': ledsEncenderApagar |= led3;
		break;
 	    case '\n':
		break;
            default:
                return -EINVAL;
        }
    }

    error = set_leds(kbd_driver, ledsEncenderApagar);
    if (error)
         return error;

    return len;
}

//MODLEDS.C
/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void){
   printk(KERN_INFO "modleds: loading\n");
   printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
   return vc_cons[fg_console].d->port.tty->driver;
}

/* Set leds to the desired value */
static inline int set_leds(struct tty_driver* handler, unsigned int mask){
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
}

static int modleds_init(void)
{
   kbd_driver= get_kbd_driver_handler();
   set_leds(kbd_driver,ALL_LEDS_ON);
   return 0;
}

static void modleds_exit(void){
    set_leds(kbd_driver,ALL_LEDS_OFF);
}

