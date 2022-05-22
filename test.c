/*
 Linux Kernel Module Baseline
*/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init test_init(void) 
{
	printk("Hello World\n");
	return 0;
}

static void __exit test_exit(void)
{
	printk("Goodbye World\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter Dinda");
MODULE_DESCRIPTION("Example Module For EECS 446");
MODULE_VERSION("0.0");

