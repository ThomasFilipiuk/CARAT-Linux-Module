/*
 * Policy Module for Address Checking 
 */

/*
 * General Approach: Keep memory regions in RB Tree nodes
 * Each node will hold <start, len, protections>
 *	-start: lower bound of the memory region
 *      -protections: set of flags (rwx or something like that)
 */

#include <linux/rbtree.h>
#include <regions.h>
/*
struct carat_memory_region_policy {
	uint64_t moduleId;
	struct rb_root *region_map;
};

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

*/
struct MemoryRegion* lookup_region(struct rb_root *root, uint64_t lookup_addr) {
       struct rb_node *n = root->rb_node;

       while (n) {
	       struct MemoryRegion *data = container_of(n, struct MemoryRegion, node);
	       
	       uint64_t start = data->addr;
	       uint64_t end = data->addr + data->len; 

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


/* 
 * STATUS
 *
 * Add functions for:
 * 	- Add region (insert in tree) DONE
 * 	- Lookup region (search in tree) DONE
 * 	- Delete region (delete in tree) DONE
 * High-Level Reqs
 * 	-Define policy struct with accept, reject, and default list
 * 	-implement protections bitfield union DONE
 * 	-turn policy into rb tree!
 * 	-interface to set policy
 * 	-test on some modules
 */	
