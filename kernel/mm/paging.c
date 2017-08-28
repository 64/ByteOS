#include "mm.h"

typedef uint64_t pte_t;

struct page_table {
	pte_t pages[512];
};

extern struct page_table p4_table; // Initial kernel p4 table

struct page_table *current_p4;

void paging_init(void) {
	current_p4 = phys_to_kern((physaddr_t)&p4_table);
}

physaddr_t paging_alloc_table(void) {
	return kern_to_phys(boot_heap_malloc(4096));
}
