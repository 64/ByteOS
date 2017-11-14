#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot2.h"
#include "system.h"

#define KERNEL_TEXT_BASE 0xFFFFFFFF80000000
#define KERNEL_LOGICAL_BASE 0xFFFF800000000000
#define KERNEL_PHYS_MAP_END 0x1000000

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER_ACCESSIBLE (1ULL << 2)
#define PAGE_WRITE_THROUGH (1ULL << 3)
#define PAGE_DISABLE_CACHE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_NO_EXEC (1ULL << 63)

#define PAGE_SIZE 4096

typedef uintptr_t physaddr_t;
typedef void *virtaddr_t;
typedef void *kernaddr_t;

typedef uint64_t pte_t;

struct page_table {
	pte_t pages[512];
} __attribute__((packed, aligned(PAGE_SIZE)));

extern struct page_table *kernel_p4;

void paging_init(void);
physaddr_t paging_get_phys_addr(struct page_table *, void *);
bool paging_has_flags(struct page_table *, void *, uint64_t flags);
pte_t paging_get_pte(struct page_table *, void *);
void paging_map_page(struct page_table *, physaddr_t, void *, uint64_t);

struct mmap_region {
	uintptr_t base;
	size_t len;
	enum mmap_region_flags {
		MMAP_NONE,
		MMAP_NOMAP
	} type;
};

void mmap_init(struct multiboot_info *);
void mmap_dump_info(void);
struct mmap_region mmap_alloc_low(size_t n);

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
