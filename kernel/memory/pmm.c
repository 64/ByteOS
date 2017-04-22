#include <klog.h>
#include <memory/pmm.h>
#include <memory/paging.h>
#include <algs/bitset.h>

struct bitset pmm_bitset;

static inline void pmm_set_addr(phys_addr a) {
	uint32_t frame = a / PAGE_SIZE;
	bitset_set(&pmm_bitset, frame);
}

static inline void pmm_clear_addr(phys_addr a) {
	uint32_t frame = a / PAGE_SIZE;
	bitset_clear(&pmm_bitset, frame);
}

static inline bool pmm_test_addr(phys_addr a) {
	uint32_t frame = a / PAGE_SIZE;
	return bitset_test(&pmm_bitset, frame);
}

void pmm_init(uintptr_t max_mem) {
	// Allocate 1 bit for each page in available memory
	bitset_create(&pmm_bitset, ((max_mem - 1) / PAGE_SIZE / 8) + 1);
}

void pmm_alloc_frame(virt_addr addr, uint32_t page_flags) {
	size_t out = 0;
	if (bitset_find_first(&pmm_bitset, &out) == 0) {
		klog_fatal("No free memory!\n");
		abort();
	}

	phys_addr allocated = out * PAGE_SIZE;
	pmm_map_frame(addr, allocated, page_flags);
}

void pmm_map_frame(virt_addr va, phys_addr pa, uint32_t page_flags) {
	pt_entry *page = paging_get(va, (page_flags & PAGE_INTERNAL_GENTABLES) != 0, NULL);

	// If the page is currently mapped, mark it as free
	if (*page & PAGE_TABLE_PRESENT)
		pmm_clear_addr(*page & 0xFFFFF000);

	*page |= PAGE_TABLE_PRESENT;
	*page |= (page_flags & PAGE_TABLE_RW) ? PAGE_TABLE_RW : 0;
	*page |= (page_flags & PAGE_TABLE_USER) ? PAGE_TABLE_USER : 0;
	*page |= PAGE_TABLE_ADDR(pa);
	pmm_set_addr(pa);
}

void pmm_free_frame(virt_addr addr) {
	pt_entry *page = paging_get(addr, 0, NULL);
	phys_addr p = *page & 0xFFFFF000;
	*page = 0;
	pmm_clear_addr(p);
}

void pmm_reserve_block(phys_addr start, size_t length) {
	phys_addr end = start + length;
	while (start < end) {
		pmm_set_addr(start);
		start += PAGE_SIZE;
	}
}

void pmm_unreserve_block(phys_addr start, size_t length) {
	phys_addr end = start + length;
	while (start < end) {
		pmm_clear_addr(start);
		start += PAGE_SIZE;
	}
}
