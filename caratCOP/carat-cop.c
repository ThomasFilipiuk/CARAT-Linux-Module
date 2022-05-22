
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define BUF_SIZE 100

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif


// set the interface parameters
/*
static int irq = 20;
module_param(irq, int, 0660);

static int mode = 1;
module_param(mode, int, 0660);
*/
static struct proc_dir_entry *ent;

/*
static ssize_t myread(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	char buf[BUF_SIZE];
	int len = 0;
	if (*ppos > 0 || count > BUF_SIZE) {
		return 0;
	}
	len += sprintf(buf, "irq=%d\n", irq);
	len += sprintf(buf + len, "mode=%d\n", mode);

	if (copy_to_user(ubuf, buf, len)) {
		return -EFAULT;
	}

	*ppos = len;
	return len;
}
*/
static ssize_t input_policy(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
        int num, c;
        char* mod_name;
        u_int64_t policy; 
	char buf[BUF_SIZE];
	if (*ppos > 0 || count > BUF_SIZE) {
		return -EFAULT;
	}

	if (copy_from_user(buf, ubuf, count)) {
		return -EFAULT;
	}

        // Upon init, each module should provide its name and a pointer to a
        // policy struct to this policy module
	num = sscanf(buf, "%s %lu", &mod_name, &policy);
	if (num != 2) {
		return -EFAULT;
	}

        
	//irq = i;
	//mode = m;
	c = strlen(buf);
	*ppos = c;
	return c;
}
#ifdef HAVE_PROC_OPS                  
static struct proc_ops myops =        
{                                     
    //.proc_read = myread,          
	.proc_write = input_policy,        
};                                    
                                      
#else                                 
static struct file_operations myops = 
{                                     
	.owner = THIS_MODULE,         
	//.read = myread,               
	.write = input_policy,             
};                                    
#endif 

static int __init test_init(void)
{
	ent = proc_create("policydev", 0660, NULL, &myops);
	printk("Hello World\n");
	return 0;
}

static void __exit test_exit(void)
{
	proc_remove(ent);
	printk("Goodbye World\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter Dinda");
MODULE_DESCRIPTION("Example Module For EECS 446");
MODULE_VERSION("0.0");

