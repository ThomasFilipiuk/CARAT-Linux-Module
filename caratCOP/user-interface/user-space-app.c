#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



#define BUF_SIZE 100
#define MOD_NAME "user-mod-test"

struct Node
{
	u_int64_t addr;
	u_int64_t len;
	u_int8_t flags;
	struct Node* next;
};

int main(int argc, char* argv[]) {

    if (argc != 2){
        printf("Usage:\n0 to test passing basic long\n" 
                "1 to test passing pointer cast as long\n"
                "2 to test passing RB tree policy struct pointer\n"); 
        return -1;
        }

    int input = atoi(argv[1]);
    printf("Running test %d\n", input);
    
    if (input == 0){
        
        char buf[BUF_SIZE];
        int len=0;
        u_int64_t policy = 5;
        
        len += sprintf(buf, "%ld", policy);

        
        int fd = open("/proc/policydev", O_RDWR);
        
	//lseek(fd, 10, SEEK_SET);
        write(fd, buf, len);
        }

    else if (input == 1){
        char buf[BUF_SIZE];
        int len=0;
        int* test_ptr = malloc(sizeof(int));
        *test_ptr = 3;

        printf("User-space-app: value: %d, pointer: %ld\n",
               *test_ptr, (u_int64_t)test_ptr);
        len += sprintf(buf, "%ld", test_ptr);

        int fd = open("/proc/policydev", O_RDWR);
	//lseek(fd, 0, SEEK_SET);
	write(fd, buf, len);
    }
    else if (input == 2){
        char buf[BUF_SIZE];
        int len=0;
        
        /* WRITE ME
         * malloc a pointer to a Node struct
         * fill in Node
         * pass pointer to module
         */
	
	struct Node *test_ptr = (struct Node *) malloc(sizeof(struct Node)); // malloc the ptr

	test_ptr->addr = 0x0000000000000001; // fill in Node
	test_ptr->len = 0x0000ffffffffffff;
	test_ptr->flags = 0x10;
	test_ptr->next = NULL;	

        len += sprintf(buf, "%ld", test_ptr);

        int fd = open("/proc/policydev", O_RDWR);
	write(fd, buf, len);
    }
    return 0;
}	
