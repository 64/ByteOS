#pragma once

#include <stdint.h>
#include <stddef.h>
#include "multiboot2.h"
#include "system.h"

#define KERNEL_TEXT_BASE 0xFFFFFFFF80000000
#define KERNEL_LOGICAL_BASE 0xFFFF800000000000
#define KERNEL_PHYS_MAP_END 0x200000

#define MEM_NORMAL 0
#define MEM_ALWAYS_MAPPED (1 << 0)

typedef uintptr_t physaddr_t;
typedef void *virtaddr_t;
typedef void *kernaddr_t;

void pmm_mmap_parse(struct multiboot_info *);

void *boot_heap_malloc(size_t n);
void *boot_heap_malloc_mapped(size_t n);
void boot_heap_free(void *p, size_t n);
void boot_heap_register_node(physaddr_t start_addr, size_t len, uint64_t flags);

void paging_init(void);

static inline physaddr_t virt_to_phys(virtaddr_t v) {
	return (physaddr_t)v - KERNEL_LOGICAL_BASE;
}

static inline virtaddr_t phys_to_virt(physaddr_t p) {
	return (virtaddr_t)(p + KERNEL_LOGICAL_BASE);
}

static inline physaddr_t kern_to_phys(kernaddr_t v) {
	return (physaddr_t)v - KERNEL_TEXT_BASE;
}

static inline kernaddr_t phys_to_kern(physaddr_t p) {
	return (kernaddr_t)(p + KERNEL_TEXT_BASE);
}
