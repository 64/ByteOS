#pragma once

#include <stdint.h>

#include "atomic.h"
#include "sync.h"
#include "limits.h"
#include "ds/linked.h"

typedef uintptr_t physaddr_t;
typedef void *virtaddr_t;
typedef void *kernaddr_t;

// One of these for each page of available physical memory.
// This should be kept as small as possible (for obvious reasons).
// Initial entries are zeroed out by memset.
struct page {
	// TODO: Add this union (will require a flags variable)
	//union {
		// PMM information
		struct {
			struct dlist_node list;
			int8_t order;
		};
		struct {
			kref_t refcount; 
			spinlock_t lock;
		};
	//};
};

extern struct page *const page_data;

static inline physaddr_t virt_to_phys(virtaddr_t v)
{
	if (v == NULL)
		return (physaddr_t)NULL;
	return (physaddr_t)v - KERNEL_LOGICAL_BASE;
}

static inline virtaddr_t phys_to_virt(physaddr_t p)
{
	if (p == (physaddr_t)NULL)
		return NULL;
	return (virtaddr_t)(p + KERNEL_LOGICAL_BASE);
}

static inline physaddr_t kern_to_phys(kernaddr_t k)
{
	if (k == NULL)
		return (physaddr_t)NULL;
	return (physaddr_t)k - KERNEL_TEXT_BASE;
}

static inline kernaddr_t phys_to_kern(physaddr_t p)
{
	if (p == (physaddr_t)NULL)
		return NULL;
	return (kernaddr_t)(p + KERNEL_TEXT_BASE);
}

static inline struct page *phys_to_page(physaddr_t p)
{
	if (p == (physaddr_t)NULL)
		return NULL;
	return (struct page *)(page_data + (p / PAGE_SIZE));
}

static inline physaddr_t page_to_phys(struct page *p)
{
	if (p == NULL)
		return (physaddr_t)NULL;
	return (physaddr_t)((p - page_data) * PAGE_SIZE);
}

static inline struct page *virt_to_page(virtaddr_t p)
{
	return phys_to_page(virt_to_phys(p));
}

static inline virtaddr_t page_to_virt(struct page *p)
{
	return phys_to_virt(page_to_phys(p));
}

