
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/rbtree.h>
#include <asm/uaccess.h>

#define BUF_SIZE 100

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif


static struct proc_dir_entry *ent;
// struct rb_root memory_map;

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

typedef struct {
	uint8_t r :1; // read
	uint8_t w :1; // write
	uint8_t x :1; // execute
	uint8_t n :1; // none
} flags_st;

union carat_region_protect_flags {                                                    
	uint8_t val;
	flags_st flags;	
};                                                                                    

struct MemoryRegion 
{
	uint64_t addr; 
	size_t len;   // in bytes

	union carat_region_protect_flags protect;

	// pthread_spinlock_t lock; // Keep a lock for concurrency control?

	struct rb_node node; // node in the rb tree
};


struct MemoryRegion* lookup_region(struct rb_root *root, uint64_t lookup_addr) {
       struct rb_node *n = root->rb_node;

       while (n) {
	       struct MemoryRegion *data = container_of(n, struct MemoryRegion, node);
	       
	       uint64_t start = data->addr;
	       uint64_t end = data->addr + data->len; // might run into an error here (uint64_t + size_t)

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

int insert_region(struct rb_root *root, struct rb_node *other) {
	struct rb_node **n = &(root->rb_node);
	struct rb_node *parent = NULL;

	while (*n) {
		struct MemoryRegion *curr_region = container_of(*n, struct MemoryRegion, node);
		struct MemoryRegion *other_region = container_of(other, struct MemoryRegion, node);

		long res = (long)curr_region->addr - (long)other_region->addr;
		parent = *n;

		if (res < 0) {
			n = &((*n)->rb_left);
		} else if (res > 0) {
			n = &((*n)->rb_right);
		} else {
			return 0;
		}
	}

	rb_link_node(other, parent, n);
	rb_insert_color(other, root);
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
	// WRITE ME
	// while list != NULL
	// 	init MemoryRegion (kmalloc)
	// 	insert_region(root, memoryRegion->node);
	// 	list = list.next;	
	while (list != NULL) {
		struct MemoryRegion* region = NULL;
		region->addr = list->addr;
		region->len = list->len;
		region->protect.val = list->flags;
		insert_region(root, &region->node);
		list = list->next;
	}

	return;
}

int check_protections(int access, flags_st flags) {
	// WRITE ME
	return 0;	
}

// Texas Guard
// Callback function for guards injected into CARATized modules
// TODO: Check permissions of address with RB tree
void texas_guard(void *ptr, int flags)
{
    printk("Checking address: 0x%x64", (unsigned long)ptr);
    // WRITE ME
    /*
    struct MemoryRegion *found = lookup_region(test_policy.region_map, (uint64_t) ptr); // need to pass in policy->memory_map
    if (found != NULL) {
    	if (!check_protections(flags, found->protect.flags)) {
     	    printk("Disallowed memory access on address: 0x%x64", (uint64_t) ptr);
	}
    	return;
    }
    */
    printk("Could not find a policy for memory address 0x%x64", (uint64_t) ptr);
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
/*
	struct rb_root *root = NULL;

	list_to_tree(root, policy);

	test_policy = (policy_t) {1, root};
*/
        // WRITE ME
        // Turn the policy linked list into a policy rbtree
        printk("policy addr: %lx", policy->addr);
        printk("policy len: %lx", policy->len);
        printk("policy addr: %x", policy->flags);
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

