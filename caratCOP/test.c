/*
 Linux Kernel Module Baseline
*/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gfp.h>

//extern void texas_guard(void *ptr, int flags);
static volatile int guard_test;

static int __init test_init(void) 
{
	printk("Hello World\n");
        guard_test = 100;
        int* test_ptr = kmalloc(sizeof(int), GFP_KERNEL);
        *test_ptr = 32;
        kvfree(test_ptr);
	return 0;
}

static void __exit test_exit(void)
{
	printk("Goodbye World\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_SOFTDEP("pre: carat-cop");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter Dinda");
MODULE_DESCRIPTION("Example Module For EECS 446");
MODULE_VERSION("0.0");

