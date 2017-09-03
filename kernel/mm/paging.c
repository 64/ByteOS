#include "libk.h"
#include "mm.h"

#define P4_ADDR_SHIFT 39
#define P3_ADDR_SHIFT 30
#define P2_ADDR_SHIFT 21
#define P1_ADDR_SHIFT 12
#define P1_ADDR_MASK (0x1FFUL << P1_ADDR_SHIFT)
#define P4_ADDR_MASK (0x1FFUL << P4_ADDR_SHIFT)
#define P3_ADDR_MASK (0x1FFUL << P3_ADDR_SHIFT)
#define P2_ADDR_MASK (0x1FFUL << P2_ADDR_SHIFT)
#define PAGE_OFFSET_MASK 0xFFFF
#define PTE_ADDR_MASK (~(0xFFF00000000001FF))

extern struct page_table p4_table; // Initial kernel p4 table

struct page_table *kernel_p4;

void paging_init(void) {
	kernel_p4 = phys_to_kern((physaddr_t)&p4_table);
}

static inline struct page_table *pgtab_extract_virt_addr(struct page_table *pgtab, uint16_t index) {
	pte_t entry = pgtab->pages[index];
	if ((entry & PAGE_PRESENT) == 0)
		return NULL;
	return phys_to_virt((entry & PTE_ADDR_MASK));
}

// TODO: More flexability with flags (e.g 'global' flag)
static inline pte_t alloc_pgtab(void) {
	// Boot heap pages are guaranteed to be mapped
	physaddr_t pgtab_phys = boot_heap_alloc_page();
	uint64_t flags = PAGE_PRESENT | PAGE_WRITABLE;
	return (pgtab_phys & PTE_ADDR_MASK) | flags;
}

static inline pte_t get_pte_from_addr(struct page_table *p4, void *addr) {
	const uintptr_t va = (uintptr_t)addr;
	const uint16_t p4_index = (va & P4_ADDR_MASK) >> P4_ADDR_SHIFT;
	const uint16_t p3_index = (va & P3_ADDR_MASK) >> P3_ADDR_SHIFT;
	const uint16_t p2_index = (va & P2_ADDR_MASK) >> P2_ADDR_SHIFT;
	const uint16_t p1_index = (va & P1_ADDR_MASK) >> P1_ADDR_SHIFT;

	struct page_table *p3_table = pgtab_extract_virt_addr(p4, p4_index);
	if (p3_table == NULL)
		return 0;

	struct page_table *p2_table = pgtab_extract_virt_addr(p3_table, p3_index);
	if (p2_table == NULL)
		return 0;

	struct page_table *p1_table = pgtab_extract_virt_addr(p2_table, p2_index);
	if (p1_table == NULL)
		return 0;

	return p1_table->pages[p1_index];
}

bool paging_has_flags(struct page_table *p4, void *addr, uint64_t flags) {
	kassert(addr != NULL);
	return (get_pte_from_addr(p4, addr) & flags) != 0;
}

void paging_map_page(struct page_table *p4, physaddr_t phys, void *virt, uint64_t flags) {
	const uintptr_t va = (uintptr_t)virt;
	const uint16_t p4_index = (va & P4_ADDR_MASK) >> P4_ADDR_SHIFT;
	const uint16_t p3_index = (va & P3_ADDR_MASK) >> P3_ADDR_SHIFT;
	const uint16_t p2_index = (va & P2_ADDR_MASK) >> P2_ADDR_SHIFT;
	const uint16_t p1_index = (va & P1_ADDR_MASK) >> P1_ADDR_SHIFT;

	struct page_table *p3_table = pgtab_extract_virt_addr(p4, p4_index);
	if (p3_table == NULL) {
		p4->pages[p4_index] = alloc_pgtab();
		p3_table = pgtab_extract_virt_addr(p4, p4_index);
		memset(p3_table, 0, sizeof(struct page_table));
	}

	struct page_table *p2_table = pgtab_extract_virt_addr(p3_table, p3_index);
	if (p2_table == NULL) {
		p3_table->pages[p3_index] = alloc_pgtab();
		p2_table = pgtab_extract_virt_addr(p3_table, p3_index);
		memset(p2_table, 0, sizeof(struct page_table));
	}


	struct page_table *p1_table = pgtab_extract_virt_addr(p2_table, p2_index);
	if (p1_table == NULL) {
		p2_table->pages[p2_index] = alloc_pgtab();
		p1_table = pgtab_extract_virt_addr(p2_table, p2_index);
		memset(p1_table, 0, sizeof(struct page_table));
	}

	p1_table->pages[p1_index] = (phys & PTE_ADDR_MASK) | PAGE_PRESENT | flags;
}

// Warning: this doesn't work for addresses below 0x1000
physaddr_t paging_get_phys_addr(struct page_table *p4, void *virt) {
	const uint16_t page_offset = (uintptr_t)virt & PAGE_OFFSET_MASK;
	const physaddr_t addr = (physaddr_t)(get_pte_from_addr(p4, virt) & PTE_ADDR_MASK);
	return (addr == 0) ? 0 : addr + page_offset;
}
