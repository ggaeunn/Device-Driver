#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>

#define CALL_DEV_NAME 	"keyled_proc"
#define CALL_DEV_MAJOR	241
#define IRQ_BUTTON_1 IRQ_EINT(1)
#define IRQ_BUTTON_2 IRQ_EINT(12)

DECLARE_WAIT_QUEUE_HEAD(WaitQueue_Read);

static unsigned long btn_no=0;
static struct proc_dir_entry *keyledproc_root_fp   = NULL;
static struct proc_dir_entry *keyledproc_led_fp    = NULL;
static char  proc_led_no = 0;
static irqreturn_t button_interrupt(int irq, void *dev_id)
{
	if(irq == 353)
		btn_no = 1;
	else
		btn_no = 2;
//	printk("btn_no : %ld\n",btn_no);
	wake_up_interruptible(&WaitQueue_Read);
	return IRQ_HANDLED;
}

static void irq_init(void)
{
	int result;
	result = request_irq(IRQ_BUTTON_1,button_interrupt,IRQF_TRIGGER_RISING,"GPIO1",NULL);
	if(result < 0)
		printk(KERN_ERR "%s : Request for IRQ %d failed\n", __FUNCTION__,IRQ_BUTTON_1);
	result = request_irq(IRQ_BUTTON_2,button_interrupt,IRQF_TRIGGER_RISING,"GPIO2",NULL);
	if(result < 0)
		printk(KERN_ERR "%s : Request for IRQ %d failed\n", __FUNCTION__,IRQ_BUTTON_2);

}
static void keyled_init(void) 
{
	int i;
	int err;
	for (i=0; i<7; i++)  //gpio led
	{
		err = gpio_request(EXYNOS4_GPK3(i), "Led");
		if (err)
			printk("led.c failed to request GPK3(%d) \n",i);
		s3c_gpio_setpull(EXYNOS4_GPK3(i), S3C_GPIO_PULL_NONE);
		gpio_direction_output(EXYNOS4_GPK3(i), 0);
	}
	err = gpio_request(EXYNOS4_GPX0(1), "key");  //gpio key
	if(err) {
		printk(KERN_ERR "%s:gpio request error(%d)!!\n", __FUNCTION__, err);
	}
	else
	{
		s3c_gpio_cfgpin(EXYNOS4_GPX0(1),S3C_GPIO_SFN(0));
		s3c_gpio_setpull(EXYNOS4_GPX0(1), S3C_GPIO_PULL_NONE);
		gpio_free(EXYNOS4_GPX0(1));
	}
	err = gpio_request(EXYNOS4_GPX1(4), "key");
	if(err) {
		printk(KERN_ERR "%s:gpio request error(%d)!!\n", __FUNCTION__, err);
	}
	else
	{
		s3c_gpio_cfgpin(EXYNOS4_GPX1(4),S3C_GPIO_SFN(0));
		s3c_gpio_setpull(EXYNOS4_GPX1(4), S3C_GPIO_PULL_NONE);
		gpio_free(EXYNOS4_GPX1(4));
	}
}
static void keyled_free(void)
{
	int i;
	for (i=0; i<7; i++)
	{
		gpio_free(EXYNOS4_GPK3(i));
	}
	free_irq(IRQ_BUTTON_1,NULL);
	free_irq(IRQ_BUTTON_2,NULL);
//  	gpio_free(EXYNOS4_GPX0(1));
//  	gpio_free(EXYNOS4_GPX1(4));
}
#if 0
static void key_read(char *buf)
{
	char temp=0;
    temp = gpio_get_value(EXYNOS4_GPX1(4));
	temp = temp << 1;
	temp = temp | gpio_get_value(EXYNOS4_GPX0(1));
	*buf = temp;
}
#endif
static void led_write(char buf)
{
	int i;
	for (i=0; i<7; i++)
	{
		gpio_direction_output(EXYNOS4_GPK3(i), (unsigned int) ((buf >> i)&0x01));
	}
}
static int keyled_open(struct inode *inode,struct file *filp)
{
	printk("call open \n");
	return 0;
}

static ssize_t keyled_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	char kbuf=0;
	int ret;
	if(!(filp->f_flags & O_NONBLOCK))
	{
		wait_event_interruptible(WaitQueue_Read,btn_no);
//		wait_event_interruptible_timeout(WaitQueue_Read,btn_no,200); //200*HZ/200 == HZ
//		if(btn_no == 0)
//			interruptible_sleep_on(&WaitQueue_Read);
	}
	
	kbuf = (char)btn_no;
	ret=copy_to_user(buf,&kbuf,count);
	btn_no = 0;
	return count;
}
static ssize_t keyled_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	char kbuf;
	int ret;
	ret=copy_from_user(&kbuf,buf,count);
	led_write(kbuf);
	return count;
}
static long keyled_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) 
{
	printk("call ioctl -> cmd : %08X, arg :%08X\n",cmd,(unsigned int)arg);
	return 0x53;
}
static unsigned int keyled_poll(struct file *filp, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	poll_wait(filp, &WaitQueue_Read, wait);
	if(btn_no > 0) mask = POLLIN ;
	return mask;
}
static int keyled_release(struct inode *inode, struct file *filp)
{
	printk("call release \n");
	return 0;
}
struct file_operations keyled_fops = 
{
	.owner	= THIS_MODULE,
	.read	= keyled_read,
	.write	= keyled_write,
	.unlocked_ioctl	= keyled_ioctl,
	.poll	= keyled_poll,
	.open	= keyled_open,
	.release	= keyled_release,
};

static char led_read(void)
{
	int i;
	unsigned long data=0;
	unsigned long temp;
	for(i=6;i>=0;i--)
	{
		temp = gpio_get_value(EXYNOS4_GPK3(i));
		data |= temp;
		if(i==0)
			break;
		data = data << 1;  //data <<= 1;
//      printk("read : %d,%x\n",i,data);
	}
	return (char)data;
}


static int read_ledproc_val( char *page, char **start, off_t off,int count,int *eof, void *data_unused )
{
	char *buf;
	char *realdata;
	realdata = (char *) data_unused;
	buf = page;
	proc_led_no = led_read();
	buf += sprintf( buf, "%d\n",  proc_led_no);
	*eof = 1;
	return buf - page;
}
static int write_ledproc_val( struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	int   len;
	char *realdata;
	realdata = (char *) data;
	if (copy_from_user(realdata, buffer, count)) return -EFAULT;
	realdata[count] = '\0';
	len = strlen(realdata);
	if (realdata[len-1] == '\n')  realdata[--len] = 0;
	proc_led_no =  simple_strtoul( realdata, NULL, 10 );
//	printk("proc test : %ld,%d,%d, %s\n",count,len,proc_led_no,realdata);
	led_write(proc_led_no);
	return count;
}
static void mkproc(void)
{
	keyledproc_root_fp  = proc_mkdir( "keyled", 0 );

	keyledproc_led_fp   = create_proc_entry( "led", S_IFREG | S_IRWXU, keyledproc_root_fp );
	if( keyledproc_led_fp )
	{
		keyledproc_led_fp->data       = &proc_led_no;
		keyledproc_led_fp->read_proc  = read_ledproc_val;
		keyledproc_led_fp->write_proc = write_ledproc_val;
	}
}
static void rmproc(void)
{
	remove_proc_entry( "led"  , keyledproc_root_fp );
	remove_proc_entry( "keyled" , 0 );
}
static int keyled_mod_init(void)
{
	int result;
	printk("call keyled_init \n");
	result = register_chrdev(CALL_DEV_MAJOR,CALL_DEV_NAME, &keyled_fops);
	if(result < 0) 
		return result;
	keyled_init();
	irq_init();
	mkproc();
	return 0;
}
static void keyled_mod_exit(void)
{
	printk("call keyled_exit \n");
	keyled_free();
	rmproc();
	unregister_chrdev(CALL_DEV_MAJOR,CALL_DEV_NAME);
}
module_init(keyled_mod_init);
module_exit(keyled_mod_exit);
MODULE_LICENSE("Dual BSD/GPL");
