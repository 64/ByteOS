#include <stddef.h>

#include "ds/linked.h"
#include "mm.h"
#include "util.h"
#include "libk.h"

#define STACK_MAX (512 - 2)
#define KERN_END ALIGNUP((physaddr_t)&_kernel_end_phys, PAGE_SIZE)

extern physaddr_t _kernel_end_phys;

struct stack_node {
	struct slist_entry list;
	int64_t current_idx; // -1 indicates no free pages in free_pages
	uintptr_t free_pages[STACK_MAX];
} __attribute__((packed)); // Shouldn't be necessary, but just to be safe

struct stack_node *stack_start;

void boot_heap_init(void) {
	// Make sure all the numbers are correct
	kassert(sizeof(struct stack_node) == PAGE_SIZE);
	kassert(KERN_END + PAGE_SIZE < KERNEL_PHYS_MAP_END);

	stack_start = phys_to_virt(KERN_END);
	slist_set_next(stack_start, list, (struct stack_node *)NULL); // Mark as end of list
	stack_start->current_idx = -1;

	// Free the pages between KERN_END + PAGE_SIZE and KERNEL_PHYS_MAP_END
	boot_heap_free_pages_phys(KERN_END + PAGE_SIZE, (KERNEL_PHYS_MAP_END - KERN_END -  PAGE_SIZE) / PAGE_SIZE);
}

void boot_heap_dump_info(void) {
	size_t n_free_pages = 0;
	slist_foreach(current_node, list, stack_start) {
		n_free_pages += current_node->current_idx + 1;
	}
	kprintf("Total nodes: %zu\n", n_free_pages / STACK_MAX);
	kprintf("Total free pages: %zu\n", n_free_pages);
}

/* Only allow 4096-length allocations ATM. Larger allocs will be tricky in the
   future since we will have to do slow O(n^2) searches of the stack. Another
   thing to consider is that this allocator causes pretty large initial
   fragmentation since we allocate right off the top of the stack. This can be
   changed in the future if physical address space fragmentation is an issue.
*/
physaddr_t boot_heap_alloc_page(void) {
	if (stack_start == NULL)
		panic("boot heap can't satisfy page allocation");

	physaddr_t free_addr = stack_start->free_pages[stack_start->current_idx];
	if (stack_start->current_idx < 0) {
		// Allocate the node itself
		free_addr = virt_to_phys((void *)stack_start);
		stack_start = slist_get_next(stack_start, list);
	} else
		stack_start->current_idx--;
	return free_addr;
}

void boot_heap_free_pages_kern(kernaddr_t k, size_t n) {
	return boot_heap_free_pages_phys(kern_to_phys(k), n);
}

void boot_heap_free_pages_virt(virtaddr_t k, size_t n) {
	return boot_heap_free_pages_phys(virt_to_phys(k), n);
}

// Must be a page aligned address
// TODO: Sort the data structure so we don't insert randomly
void boot_heap_free_pages_phys(physaddr_t p, size_t n) {
	physaddr_t free_start = p;
	physaddr_t free_end = p + (n * PAGE_SIZE);

	kassert(n != 0);
	kassert(p >= KERN_END && (p & 0xFFF) == 0);

	// TODO: Please test this!
	if (stack_start == NULL) {
		kprintf("stack_start is null, reallocating...\n");
		pte_t pte = paging_get_pte(kernel_p4, phys_to_virt(free_start));
		// Using this address requires allocation to map, but we have no space
		if (pte == 0)
			panic("boot heap cannot create space to store free pages");
		// It's not mapped, but we can map it in since it requires no allocation
		else if ((pte & PAGE_PRESENT) == 0)
			paging_map_page(kernel_p4, free_start, phys_to_virt(free_start), PAGE_WRITABLE | PAGE_NO_EXEC);
		// Otherwise it's now mapped so we can use it
		stack_start = phys_to_virt(free_start);
		stack_start->current_idx = -1;
		free_start += PAGE_SIZE;
	}

	kprintf("Freeing physical pages from %p to %p\n", (void *)free_start, (void *)free_end);
	for (; free_start < free_end; free_start += PAGE_SIZE) {
		paging_map_page(kernel_p4, free_start, phys_to_virt(free_start), PAGE_WRITABLE | PAGE_NO_EXEC);
		if (stack_start->current_idx + 1 >= STACK_MAX) {
			// Need new stack node
			struct stack_node *next_node = phys_to_virt(boot_heap_alloc_page());
			next_node->current_idx = 0;
			next_node->free_pages[0] = free_start;
			slist_set_next(next_node, list, stack_start);
			stack_start = next_node;
		} else {
			// Use existing stack node
			stack_start->current_idx++;
			stack_start->free_pages[stack_start->current_idx] = free_start;
		}
	}
}
