#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <system.h>
#include <algs/oarray.h>

#define KHEAP_START        0x60000000
#define KHEAP_INITIAL_SIZE 0x100000
#define KHEAP_MAX 	   0x300000
#define KHEAP_INDEX_SIZE   0x20000
#define KHEAP_MAGIC        0x123890AB
#define KHEAP_MIN_SIZE     0x70000

extern phys_addr placement_address;

struct kheap_header {
	uint32_t magic;
	bool is_hole;
	size_t size;
};

struct kheap_footer {
	uint32_t magic;
	struct kheap_header *header;
};

struct kheap_heap {
	struct oarray index;
	uintptr_t start_addr;
	uintptr_t end_addr;
	uintptr_t max_addr;
	bool supervisor;
	bool readonly;
};

extern struct kheap_heap *kheap;

struct kheap_heap *kheap_create(uintptr_t start, uintptr_t end, uintptr_t max, bool supervisor, bool readonly);
void *kheap_alloc(size_t size, bool page_align, struct kheap_heap *heap);
void kheap_free(void *p, struct kheap_heap *heap);

virt_addr kmalloc_internal(size_t size, bool align, phys_addr *phys); // Internal use only.
virt_addr kmalloc_a(size_t size);  // Page aligned.
virt_addr kmalloc_p(size_t size, phys_addr *phys); // Returns a physical address.
virt_addr kmalloc_ap(size_t size, phys_addr *phys); // Page aligned and returns a physical address.
virt_addr kmalloc(size_t size); // Vanilla (normal).
void kfree(void *p);
