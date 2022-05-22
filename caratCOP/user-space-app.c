#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 100
#define MOD_NAME "user-mod-test"

int main(void) {
    /*
	char buf[100];
	int fd = open("/proc/mydev", O_RDWR);
	read(fd, buf, 100);
	puts(buf);
	
	lseek(fd, 0, SEEK_SET);
	write(fd, "33 4", 2);

	lseek(fd, 0, SEEK_SET);
	read(fd, buf, 100);
	puts(buf);
    */

        printf("User-space-app: Hello World\n");

        char buf[BUF_SIZE];
        int len=0;
        
        int data[5] = {1, 2, 3, 4, 5};
        uint64_t data_ptr = (uint64_t)&data[0];

        len += sprintf(buf, "%s ", MOD_NAME);
        len += sprintf(buf + len, "%lu", data_ptr);
        
	int fd = open("/proc/policydev", O_RDWR);
	lseek(fd, 0, SEEK_SET);
	write(fd, buf, len);


}	
