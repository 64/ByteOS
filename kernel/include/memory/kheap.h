#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <system.h>

#define HDR_SIZE (sizeof(union kheap_hdr))
#define MIN_MORECORE_UNITS ((4096 / (HDR_SIZE)) * 2)
#define KHEAP_START 0x50000000
#define KHEAP_MAX 0x50100000
#define KHEAP_AA_START 0x50100000
#define KHEAP_AA_MAX 0x50200000

union kheap_hdr {
	struct {
		union kheap_hdr *ptr;
		size_t size;
	} s;
	uint64_t align;
};

typedef union kheap_hdr kheap_hdr;

extern phys_addr placement_address;

void kheap_init();
void *kheap_alloc(size_t size, bool page_align);
void kheap_free(void *p);

virt_addr kmalloc_internal(size_t size, bool align, phys_addr *phys); // Internal use only.
void *kmalloc_a(size_t size);  // Page aligned.
void *kmalloc_p(size_t size, phys_addr *phys); // Returns a physical address.
void *kmalloc_ap(size_t size, phys_addr *phys); // Page aligned and returns a physical address.
void *kmalloc(size_t size); // Vanilla (normal).
void kfree(void *p);
