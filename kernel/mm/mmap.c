#include <stddef.h>
#include <stdint.h>
#include "libk.h"
#include "mm.h"
#include "multiboot2.h"
#include "util.h"

#define MAX_REGIONS 128

struct mmap_type {
	uint32_t count;
	struct mmap_region *regions;
};

struct mmap {
	physaddr_t highest_mapped;
	struct mmap_type available;
	struct mmap_type reserved;
};

struct mmap_region available[MAX_REGIONS];
struct mmap_region reserved[MAX_REGIONS];

struct mmap mem_map = {
	.available = { .count = 0, .regions = available },
	.reserved = { .count = 0, .regions = reserved }
};

// TODO: Insertion sort
static void mmap_insert_region(struct mmap_type *type, uintptr_t addr, size_t len, enum mmap_region_flags flags) {
	kassert(type->count < MAX_REGIONS);
	type->regions[type->count++] = (struct mmap_region){
		.base = addr,
		.len = len,
		.type = flags
	};
}

static void mmap_delete_region(struct mmap_type *type, size_t i) {
	kassert_dbg(i < type->count);
	memmove(type->regions + i, type->regions + i + 1, (type->count - i - 1) * sizeof(struct mmap_region));
	type->count--;
}

void mmap_dump_info(void) {
	kprintf("Highest mapped physical page: %p\n", (void *)mem_map.highest_mapped);
	kprintf("Available:\n");
	for (size_t i = 0; i < mem_map.available.count; i++)
		kprintf("\tRegion base %p, len %zu\n", (void *)mem_map.available.regions[i].base, mem_map.available.regions[i].len);
	kprintf("Reserved:\n");
	for (size_t i = 0; i < mem_map.reserved.count; i++)
		kprintf("\tRegion base %p, len %zu\n", (void *)mem_map.reserved.regions[i].base, mem_map.reserved.regions[i].len);
}

struct mmap_region mmap_alloc_low(size_t n) {
	for (size_t i = 0; i < mem_map.available.count; i++) {
		struct mmap_region *rg = &mem_map.available.regions[i];
		// TODO: Cleanup
		size_t available_bytes;
		if (rg->base < mem_map.highest_mapped)
			available_bytes = MIN(mem_map.highest_mapped - rg->base, rg->len);
		else
			available_bytes = rg->len;

		// If we can satisfy the allocation by splitting into a new block
		if (n <= available_bytes) {
			size_t remaining = available_bytes - n;
			uintptr_t new_base = rg->base + n;
			struct mmap_region old_rg = { .base = rg->base, .len = n };
			mmap_delete_region(&mem_map.available, i);
			mmap_insert_region(&mem_map.available, new_base, remaining, rg->type);
			return old_rg;
		}
	}
	// Allocation not satisfied
	panic("Cannot satisfy mmap allocation of size %zu", n);
}

// TODO: Cleanup
void mmap_init(struct multiboot_info *mboot) {
	// Find memory map
	struct multiboot_tag_mmap *p = (struct multiboot_tag_mmap*)mboot->tags;
	for (; p->type != MULTIBOOT_TAG_TYPE_END && p->type != MULTIBOOT_TAG_TYPE_MMAP;
			p = (struct multiboot_tag_mmap*)ALIGNUP((uint64_t)p + p->size, 8))
		;
	if (p->type != MULTIBOOT_TAG_TYPE_MMAP)
		panic("No multiboot memory map tag found");
		
	uint32_t entry_size = p->entry_size;
	uint32_t total_size = p->size;

	struct multiboot_mmap_entry *mmap;
	// Loop over memory map
	for (mmap = p->entries; (uint8_t*)mmap < (uint8_t*)p + total_size;
                      mmap = (struct multiboot_mmap_entry*)((uintptr_t)mmap + entry_size))
	{
		// Insert a new region for each node in the memory map
		struct mmap_type *target;
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
			target = &mem_map.available;
		else
			target = &mem_map.reserved;
		mmap_insert_region(target, mmap->addr, mmap->len, MMAP_NONE);
	}


	mem_map.highest_mapped = KERNEL_PHYS_MAP_END;
	mmap_dump_info();
}
