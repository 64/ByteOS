#pragma once

#include <stdint.h>
#include <stddef.h>

#include "limits.h"
#include "smp.h"
#include "sync.h"
#include "atomic.h"
#include "addr.h"
#include "ds/linked.h"

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

// Describes a contiguous block of memory
struct zone {
	struct slist_node list;
	physaddr_t pa_start; // Start of available memory in zone
	size_t len; // Length of available memory in zone
	struct page *free_lists[MAX_ORDER];
};

// Describes a region of virtual memory
struct vm_area {
	struct slist_node list;
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
	struct page_table *p4; // This should stay as the first member
	rwspinlock_t pgtab_lock;

	// users: Number of threads with this mmu_info set as active
	kref_t users;

	spinlock_t cpu_lock;
	cpuset_t cpus;

	spinlock_t area_lock;
	struct vm_area *areas;
};

struct tlb_op {
	virtaddr_t start, end;
};

