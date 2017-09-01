#include <stddef.h>

#include "ds/linked.h"
#include "mm.h"
#include "util.h"
#include "libk.h"

#define BLOCK 4096
#define STACK_IDX_MAX (512 - 2)
#define GET_PAGE_PTR(node, idx) ((uintptr_t *)(node) + 2 + (idx))

extern physaddr_t _kernel_end_phys;

struct stack_node {
	struct slist_entry *list;
	size_t current_idx;
	uintptr_t free_pages[STACK_IDX_MAX];
};

struct stack_node *stack_start;

void boot_heap_init(void) {
	// Map the unused space that we already have allocated since boot
	extern physaddr_t _kernel_end_phys;
	const physaddr_t kern_end = ALIGNUP((physaddr_t)&_kernel_end_phys, 4096);
	stack_start = phys_to_kern(kern_end);
	slist_set_next(stack_start, list, NULL);
	memset(free_pages, 0, sizeof(free_pages));
	for (stack_start->current_idx = 0; stack_start->current_idx < STACK_IDX_MAX; stack_start->current_idx++) {
		// Do magic
	}
}

void boot_heap_register_node(physaddr_t start_addr, size_t len) {

}

static void *_boot_heap_alloc(size_t n, uint64_t alloc_flags) {
	n = ALIGNUP(n, BLOCKSIZE);
	panic("boot heap can't satisfy allocation of size %zu", n);
}

void *boot_heap_malloc(size_t n) {
	return _boot_heap_alloc(n, 0);
}

void boot_heap_free(void  *p, size_t n) {
	n = ALIGNUP(n, BLOCKSIZE);
	panic("boot heap can't free allocation of size %zu", n);
}
