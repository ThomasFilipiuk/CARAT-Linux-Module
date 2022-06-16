#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



#define BUF_SIZE 1000
#define MOD_NAME "user-mod-test"

#define READABLE 1
#define WRITABLE 2
#define EXECUTABLE 4

typedef struct Node
{
	u_int64_t addr;
	u_int64_t len;
	u_int8_t flags;
	struct Node* next;
} node_t;

int main(int argc, char* argv[]) {

    if (argc != 2){
        printf("Usage:\n0 to test passing basic long\n" 
                "1 to test passing pointer cast as long\n"
                "2 to test passing RB tree policy struct pointer\n"
                "3 to test passing policy with multiple nodes\n"
                "4 to test reading policy from module\n"); 
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
        
        /* 
         * malloc a pointer to a Node struct
         * fill in Node
         * pass pointer to module
         */
	
	node_t *test_ptr = (node_t*) malloc(sizeof(node_t)); // malloc the ptr

	test_ptr->addr = 0x0000000000000000; // fill in Node
	test_ptr->len = 0xffffffffffffffff; // entire address space
	test_ptr->flags = READABLE | EXECUTABLE;
	test_ptr->next = NULL;	

        len += sprintf(buf, "%ld", test_ptr);

        int fd = open("/proc/policydev", O_RDWR);
	write(fd, buf, len);
    }
    else if (input == 3){
        char buf[BUF_SIZE];
        int len=0;
        
        /* 
         * malloc a pointers to Node structs
         * create in linked list
         * pass head pointer to module
         */
	
	node_t *test_ptr = (node_t*) malloc(sizeof(node_t)); 
        node_t *test_ptr_2 = (node_t*) malloc(sizeof(node_t));
        node_t *test_ptr_3 = (node_t*) malloc(sizeof(node_t));
        node_t *test_ptr_4 = (node_t*) malloc(sizeof(node_t));
        node_t *test_ptr_5 = (node_t*) malloc(sizeof(node_t));

        // Bottom 1/16 of addresses have no permissions
	test_ptr->addr = 0x0000000000000000; 
	test_ptr->len = 0x0fffffffffffffff; 
	test_ptr->flags = 0;
	test_ptr->next = test_ptr_2;

        // Rest of first 1/2 of addresses have full permissions
        test_ptr_2->addr = 0x1000000000000000; 
	test_ptr_2->len = 0x6fffffffffffffff; 
	test_ptr_2->flags = READABLE | WRITABLE | EXECUTABLE;
	test_ptr_2->next = test_ptr_3;

        // Addresses 0x8000... to 0xF000... have write permissions
        test_ptr_3->addr = 0x8000000000000000; 
	test_ptr_3->len = 0x6fffffffffffffff; 
	test_ptr_3->flags = WRITABLE;
	test_ptr_3->next = test_ptr_4;

        // Addresses 0xF00... to 0xFFFFFFFF00000000 - 1 have read permissions
        test_ptr_4->addr = 0xF000000000000000; 
	test_ptr_4->len = 0x0FFFFFFEFFFFFFFF; 
	test_ptr_4->flags = READABLE;
	test_ptr_4->next = test_ptr_5;

        // The top addresses have execute permissions
        test_ptr_5->addr = 0xFFFFFFFF00000000; 
	test_ptr_5->len = 0x00000000FFFFFFFF; 
	test_ptr_5->flags = EXECUTABLE;
	test_ptr_5->next = NULL;


        

        len += sprintf(buf, "%ld", test_ptr);

        int fd = open("/proc/policydev", O_RDWR);
	write(fd, buf, len);
    }
    else if (input == 4){
        char buf[BUF_SIZE];
        u_int64_t output;
        node_t* policy;
        int num, region_num = 0;
        int fd = open("/proc/policydev", O_RDWR);
        
	read(fd, buf, BUF_SIZE);
        printf(buf);
        /*num = sscanf(buf, "%lx", &output); 
        if (num != 1) {
            printf("Scan failed");
            return -1;
        }

        policy = (node_t*)output;
        printf("pointer: %lx\n", policy); 
        while (policy){
            printf("Region %d\n"
                   "Address: %lx\n"
                   "Length: %lx\n"
                   "Flags: %d\n",
                   region_num,
                   policy->addr,
                   policy->len,
                   policy->flags);
            region_num++;
            policy = policy->next;
        }
*/      

    }
    
    return 0;
}	
