#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot2.h"
#include "types.h"
#include "spin.h"
#include "ds/linked.h"

#define KERNEL_TEXT_BASE 0xFFFFFFFF80000000
#define KERNEL_PHYS_MAP_END 0x1000000
#define KERNEL_LOGICAL_BASE 0xFFFF800000000000
#define KERNEL_PAGE_DATA (KERNEL_TEXT_BASE + KERNEL_PHYS_MAP_END)

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER_ACCESSIBLE (1ULL << 2)
#define PAGE_WRITE_THROUGH (1ULL << 3)
#define PAGE_DISABLE_CACHE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_COW (1ULL << 9)
#define PAGE_EXECUTABLE (1ULL << 63)

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PTE_ADDR_MASK (~(0xFFF0000000000FFFUL))

#define VMM_ALLOC_MMAP (1 << 0)

#define MMAP_ALLOC_PA (1 << 0)
#define MMAP_MAX_REGIONS 128

#define GFP_CAN_FAIL (1 << 0)

// This means that the largest allocation is 2^(MAX_ORDER - 1) * 4096 bytes
#define MAX_ORDER 12

#define GFP_NONE 0
#define VMM_NONE 0
#define KM_NONE 0

typedef uint64_t pte_t;

struct page_table {
	pte_t pages[512];
} __attribute__((packed, aligned(PAGE_SIZE)));

struct mmap_region {
	uintptr_t base;
	size_t len;
	// TODO: Don't name this flags, it's an enum
	enum mmap_region_flags {
		MMAP_NONE,
		MMAP_NOMAP
	} flags;
};

struct mmap_type {
	uint32_t count;
	struct mmap_region *regions;
};

struct mmap {
	physaddr_t highest_mapped;
	struct mmap_type available;
	struct mmap_type reserved;
};

// This should be kept as small as possible
// Initial entries are zeroed out by memset
struct page {
	struct dlist_entry list; // Can be used for whatever purpose
	uint32_t refcount; // Reference count for copy-on-write mappings
	int8_t order; // For buddy allocator system
};

// Describes a contiguous block of memory
struct zone {
	struct slist_entry list;
	physaddr_t pa_start; // Start of available memory in zone
	size_t len; // Length of available memory in zone
	struct page *free_lists[MAX_ORDER];
};

// Describes a region of virtual memory
struct vm_area {
	struct slist_entry list;
	uintptr_t base;
	size_t len;
	uint32_t type;
#define VM_AREA_OTHER 0
#define VM_AREA_STACK 1
#define VM_AREA_TEXT 2
#define VM_AREA_BSS 3
	uint32_t flags;
#define VM_AREA_WRITABLE (1 << 0)
#define VM_AREA_EXECUTABLE (1 << 1)
};

struct mmu_info {
	struct page_table *p4;
	struct vm_area *areas;
};

extern struct mmu_info kernel_mmu;
extern const uintptr_t _kernel_end_phys;
extern struct page * const page_data;

void vmm_init(void);
void vmm_map_all(struct mmap *);
physaddr_t vmm_get_phys_addr(struct mmu_info *, void *);
bool vmm_has_flags(struct mmu_info *, void *, uint64_t flags);
pte_t *vmm_get_pte(struct mmu_info *, void *);
void vmm_map_page(struct mmu_info *, physaddr_t, virtaddr_t, unsigned long);
void vmm_dump_tables(void);
void vmm_destroy_low_mappings(struct mmu_info *);

struct mmap *mmap_init(struct multiboot_info *);
void mmap_dump_info(void);
struct mmap_region mmap_alloc_low(size_t n, unsigned int alloc_flags);

void pmm_init(struct mmap *);
struct page *pmm_alloc_order(unsigned int order, unsigned int alloc_flags) __attribute__((warn_unused_result));
void pmm_free_order(struct page *page, unsigned int order);

void *kmalloc(size_t, unsigned int) __attribute__((malloc));
void kfree(void *);

void cow_copy_pte(pte_t *dest, pte_t *src);
bool cow_handle_write(pte_t *pte);

static inline physaddr_t virt_to_phys(virtaddr_t v) {
	if (v == NULL)
		return (physaddr_t)NULL;
	return (physaddr_t)v - KERNEL_LOGICAL_BASE;
}

static inline virtaddr_t phys_to_virt(physaddr_t p) {
	if (p == (physaddr_t)NULL)
		return NULL;
	return (virtaddr_t)(p + KERNEL_LOGICAL_BASE);
}

static inline physaddr_t kern_to_phys(kernaddr_t k) {
	if (k == NULL)
		return (physaddr_t)NULL;
	return (physaddr_t)k - KERNEL_TEXT_BASE;
}

static inline kernaddr_t phys_to_kern(physaddr_t p) {
	if (p == (physaddr_t)NULL)
		return NULL;
	return (kernaddr_t)(p + KERNEL_TEXT_BASE);
}

static inline struct page *phys_to_page(physaddr_t p) {
	if (p == (physaddr_t)NULL)
		return NULL;
	return (struct page *)(page_data + (p / PAGE_SIZE));
}

static inline physaddr_t page_to_phys(struct page *p) {
	if (p == NULL)
		return (physaddr_t)NULL;
	return (physaddr_t)((p - page_data) * PAGE_SIZE);
}

static inline struct page *virt_to_page(virtaddr_t p) {
	return phys_to_page(virt_to_phys(p));
}

static inline virtaddr_t page_to_virt(struct page *p) {
	return phys_to_virt(page_to_phys(p));
}
