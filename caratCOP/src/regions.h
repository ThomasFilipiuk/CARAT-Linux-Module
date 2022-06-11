/*
 *  * Policy Module for Address Checking
 *   */

/*
 *  * General Approach: Keep memory regions in RB Tree nodes
 *   * Each node will hold <start, len, protections>
 *    *      -start: lower bound of the memory region
 *     *      -protections: set of flags (rwx or something like that)
 *      */

#include <linux/rbtree.h>

struct carat_memory_region_policy {
	uint64_t moduleId;
	struct rb_root *region_map;
};

typedef struct {
	uint8_t r :1; // read
	uint8_t w :1; // write
	int8_t x :1; // execute
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

	// pthread_spinlock_t lock; // Keep a lock for concurrency control
	struct rb_node node; // node in the rb tree
};

struct MemoryRegion* lookup_region(struct rb_root *root, uint64_t lookup_addr);

int insert_region(struct rb_root *root, struct rb_node *other);

int delete_region(struct rb_root *root, uint64_t addr);
