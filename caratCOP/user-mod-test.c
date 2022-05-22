#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 100
#define MOD_NAME "user-mod-test"

static int __init test_init(void)
{
    

        printk("User-space-app: Hello World\n");

        char buf[BUF_SIZE];
        int len=0;
        
        int data[5] = {1, 2, 3, 4, 5};
        data = (u_int64_t)data;

        len += sprintf(buf, "%s ", MOD_NAME);
        len += sprintf(buf + len, "lu", data);
        
	int fd = open("/proc/mydev", O_RDWR);
	lseek(fd, 0, SEEK_SET);
	write(fd, buf, len);


	return 0;
}

static void __exit test_exit(void)
{
	printk("User-module-test: Goodbye World\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter Dinda");
MODULE_DESCRIPTION("Example Module For EECS 446");
MODULE_VERSION("0.0");
