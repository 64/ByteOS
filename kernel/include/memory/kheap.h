#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <algs/oarray.h>

#define KHEAP_START        0x50000000
#define KHEAP_INITIAL_SIZE 0x100000
#define KHEAP_MAX 	   0x1000000
#define KHEAP_INDEX_SIZE   0x20000
#define KHEAP_MAGIC        0x123890AB
#define KHEAP_MIN_SIZE     0x70000

extern uintptr_t placement_address;

struct kheap_header {
	uint32_t magic;
	bool is_hole;
	uint32_t size;
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
void *kheap_alloc(uint32_t size, bool page_align, struct kheap_heap *heap);
void kheap_free(void *p, struct kheap_heap *heap);

uintptr_t kmalloc_internal(uint32_t size, bool align, uint32_t *phys); // Internal use only.
uintptr_t kmalloc_a(uint32_t size);  // Page aligned.
uintptr_t kmalloc_p(uint32_t size, uint32_t *phys); // Returns a physical address.
uintptr_t kmalloc_ap(uint32_t size, uint32_t *phys); // Page aligned and returns a physical address.
uintptr_t kmalloc(uint32_t size); // Vanilla (normal).
void kfree(void *p);
