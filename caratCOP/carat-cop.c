
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/rbtree.h>
#include <asm/uaccess.h>

#define BUF_SIZE 100
#define READABLE 1
#define WRITABLE 2
#define EXECUTABLE 4

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif


static struct proc_dir_entry *ent;


/*
 *
 * POLICY STRUCT IMPLEMENTATION
 *
 */

/*
 * Policy Module for Address Checking 
 */

/*
 * General Approach: Keep memory regions in RB Tree nodes
 * Each node will hold <start, len, protections>
 *	-start: lower bound of the memory region
 *      -protections: set of flags (rwx or something like that)
 */


typedef struct carat_memory_region_policy {
	uint64_t moduleId;
	struct rb_root *region_map;
} policy_t;

// Global variable for storing the policy of test.ko
policy_t test_policy;
/* Using bitfields won't work because the location of the fields aren't deterministic
typedef struct {
	uint8_t r :1; // read
	uint8_t w :1; // write
	uint8_t x :1; // execute
	uint8_t n :1; // none
} flags_st;

typedef union carat_region_protect_flags {                                             
	uint8_t val;
	flags_st flags;	
} protect_flags_t;
*/

typedef struct MemoryRegion 
{
	uint64_t addr; 
	size_t len;   // in bytes

	u_int8_t protect;

	// pthread_spinlock_t lock; // Keep a lock for concurrency control?

	struct rb_node node; // node in the rb tree
} memory_region_t;


memory_region_t* lookup_region(struct rb_root *root, uint64_t lookup_addr) {
       struct rb_node *n = root->rb_node;
       printk("Looking up address %lx\n", lookup_addr); 
       while (n) {
	       memory_region_t *data = container_of(n, memory_region_t, node);
	       
	       uint64_t start = data->addr;
	       uint64_t end = data->addr + data->len;
               printk("start: %lx\nend: %lx", start, end);

	       if (lookup_addr < start) {
		       n = n->rb_left;
	       } else if (lookup_addr >= end) {
		       n = n->rb_right;
	       } else {
		       return data;
	       }
       }

       return NULL; // if nothing was found :(
}       

int insert_region(struct rb_root *root, memory_region_t* new_region) {
	struct rb_node **n = &(root->rb_node);
	struct rb_node *parent = NULL;
        long other_addr = new_region->addr;
        printk("Begin inserting region\n");
        while (*n) {
            printk("Looping\n");
		memory_region_t *curr_region = container_of(*n, memory_region_t, node);

		long res = (long)curr_region->addr - other_addr;
		parent = *n;

		if (res < 0) {
			n = &((*n)->rb_left);
		} else if (res > 0) {
			n = &((*n)->rb_right);
		} else {
			return 0;
		}
	}
        printk("End of while loop\n");
	rb_link_node(&new_region->node, parent, n);
	rb_insert_color(&new_region->node, root);
	return 1;
}

int delete_region(struct rb_root *root, uint64_t addr) {
	struct MemoryRegion *found_region = lookup_region(root, addr);
	if (found_region) {
		rb_erase(&found_region->node, root);
		// free memory used by data?
		return 1;
	}
	return 0;
}

// This is the format that will be used to pass in the policy from user space

// could we maybe just put it in a separate file and link to it?
typedef struct Node
{
	u_int64_t addr;
	u_int64_t len;
	u_int8_t flags;
	struct Node* next;
} node_t;

void list_to_tree(struct rb_root *root, node_t *list) {
		
	while (list != NULL) {
                memory_region_t* region = kmalloc(sizeof(memory_region_t), GFP_KERNEL);
		region->addr = list->addr;
		region->len = list->len;
		region->protect = list->flags;
		insert_region(root, region);
		list = list->next;
	}

	return;
}

int check_protections(int access, u_int8_t flags) {
    // Mask access flags against protect flags, compare to original access flags
    return (access & flags) == access;	
}

// Texas Guard
// Callback function for guards injected into CARATized modules
// TODO: Check permissions of address with RB tree
void texas_guard(void *ptr, int flags)
{
    uint64_t addr = (uint64_t) ptr;
    printk("Checking address: %lx", addr);

    // need to pass in policy->memory_map
    memory_region_t *found = lookup_region(test_policy.region_map, addr);
    if (found) {
    	if (!check_protections(flags, found->protect)) {
     	    printk("Disallowed memory access on address: %lx", addr);
	}
        else {
            printk("Memory access allowed at addres: %lx", addr);
        }
    	return;
    }
    
    printk("Could not find a policy for memory address %lx", addr);
    return;
}

EXPORT_SYMBOL(texas_guard);


// Input policy
// Provides a way for the user to pass a policy to this module via procfs interface
// TODO: Test interface, possibly switch to ioctl instead
static ssize_t input_policy(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
        int num, c;
        u_int64_t user_input;
        node_t* policy;
	char buf[BUF_SIZE];
        struct rb_root* root;
        
	if (*ppos > 0 || count > BUF_SIZE) {
		return -EFAULT;
	}

	if (copy_from_user(buf, ubuf, count)) {
		return -EFAULT;
	}

        // Upon init, user should provide a pointer to a
        // policy struct to this policy module
	num = sscanf(buf, "%ld", &user_input);
	if (num != 1) {
		return -EFAULT;
	}

        policy = (node_t*)user_input;

	root = kmalloc(sizeof(struct rb_root), GFP_KERNEL);
        *root = RB_ROOT;
        
	list_to_tree(root, policy);

	test_policy = (policy_t){ .moduleId=1, .region_map=root };

        // WRITE ME
        // Turn the policy linked list into a policy rbtree
        printk("policy addr: %lx", policy->addr);
        printk("policy len: %lx", policy->len);
        printk("policy flags: %x", policy->flags);
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

