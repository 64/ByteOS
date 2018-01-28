#include <stddef.h>
#include <stdint.h>
#include "libk.h"
#include "mm.h"
#include "multiboot2.h"
#include "util.h"

#define MAX_REGIONS 128
#define WITHIN(x, y, len) ((x) <= (y) && ((x) + (len) > (y)))

// These could be dynamically resizable in future
struct mmap_region available[MAX_REGIONS];
struct mmap_region reserved[MAX_REGIONS];

static struct mmap mem_map = {
	.available = { .count = 0, .regions = available },
	.reserved = { .count = 0, .regions = reserved }
};

static void mmap_delete_region(struct mmap_type *type, size_t i)
{
	kassert_dbg(i < type->count);
	memmove(type->regions + i, type->regions + i + 1, (type->count - i - 1) * sizeof(struct mmap_region));
	type->count--;
}

static void mmap_insert_region(struct mmap_type *type, uintptr_t addr, size_t len, enum mmap_region_flags flags)
{
	kassert(type->count <= MAX_REGIONS);
	if (len == 0)
		return;
	// Search for correct position in region list
	for (size_t i = 0; i < type->count; i++) {
		uintptr_t rg_addr = type->regions[i].base;
		size_t rg_len = type->regions[i].len;
		// If the region is directly below, merge
		if (addr + len == rg_addr) {
			type->regions[i].base = addr;
			type->regions[i].len += len;
		}
		// If the region is below, insert
		else if (addr + len < rg_addr) {
			if (type->count == MAX_REGIONS)
				panic("No space left in mmap");
			// Shift everything to the right
			memmove(type->regions + i + 1, type->regions + i, (type->count - i) * sizeof(struct mmap_region));
			const struct mmap_region new_region = {
				.base = addr,
				.len = len,
				.flags = flags
			};
			type->regions[i] = new_region;
			type->count++;
			return;
		}
		// If the region is directly above, merge
		else if (rg_addr + rg_len == addr) {
			type->regions[i].len += len;
			// Check if we can merge the next block
			if (i < type->count - 1 && addr + len == type->regions[i + 1].base) {
				type->regions[i].len += type->regions[i + 1].len;
				mmap_delete_region(type, i + 1);
			}
			return;
		}
		// If the memory overlaps, something bad happened
		kassert_dbg(!WITHIN(addr, rg_addr, len) && !WITHIN(rg_addr, addr, rg_len));
	}

	// Special case: we are inserting a region after all other regions
	if (type->count == MAX_REGIONS)
		panic("No space left in mmap");
	const struct mmap_region new_region = {
		.base = addr,
		.len = len,
		.flags = flags
	};
	type->regions[type->count++] = new_region;
}

void mmap_dump_info(void)
{
	kprintf("Highest mapped physical page: %p\n", (void *)mem_map.highest_mapped);
	kprintf("Available:\n");
	for (size_t i = 0; i < mem_map.available.count; i++)
		kprintf("\tRegion base %p, len %zu\n", (void *)mem_map.available.regions[i].base, mem_map.available.regions[i].len);
	kprintf("Reserved:\n");
	for (size_t i = 0; i < mem_map.reserved.count; i++)
		kprintf("\tRegion base %p, len %zu\n", (void *)mem_map.reserved.regions[i].base, mem_map.reserved.regions[i].len);
}

struct mmap_region mmap_alloc_low(size_t n, unsigned int alloc_flags)
{
	for (size_t i = 0; i < mem_map.available.count; i++) {
		struct mmap_region *rg = &mem_map.available.regions[i];
		if (rg->base >= mem_map.highest_mapped)
			break; // This memory is above what is mapped, so give up

		size_t available_bytes, effective_len = rg->len;
		uintptr_t effective_base = rg->base;

		if (alloc_flags & MMAP_ALLOC_PA) {
			effective_base = ALIGNUP(rg->base, PAGE_SIZE);
			effective_len -= (effective_base - rg->base);
		}

		// If we can satisfy the allocation by splitting into a new block
		available_bytes = MIN(mem_map.highest_mapped - effective_base, effective_len);
		if (n <= available_bytes) {
			size_t remaining = available_bytes - n;
			uintptr_t new_base = effective_base + n;
			struct mmap_region old_rg = { .base = effective_base, .len = n, .flags = rg->flags };
			if (alloc_flags & MMAP_ALLOC_PA) {
				uintptr_t padded_base = rg->base;
				size_t padded_len = effective_base - rg->base;
				mmap_delete_region(&mem_map.available, i);
				mmap_insert_region(&mem_map.available, padded_base, padded_len, rg->flags);
			} else
				mmap_delete_region(&mem_map.available, i);
			mmap_insert_region(&mem_map.reserved, old_rg.base, old_rg.len, old_rg.flags);
			if (remaining > 0)
				mmap_insert_region(&mem_map.available, new_base, remaining, rg->flags);
			return old_rg;
		}
	}
	// Allocation not satisfied
	panic("Cannot satisfy mmap allocation of size %zu", n);
}

// TODO: Cleanup
struct mmap *mmap_init(struct multiboot_info *mboot)
{
	// Find memory map
	struct multiboot_tag_mmap *p = (struct multiboot_tag_mmap *)mboot->tags;
	for (; p->type != MULTIBOOT_TAG_TYPE_END && p->type != MULTIBOOT_TAG_TYPE_MMAP;
	     p = (struct multiboot_tag_mmap *)ALIGNUP((uintptr_t)p + p->size, 8))
		;
	if (p->type != MULTIBOOT_TAG_TYPE_MMAP)
		panic("No multiboot memory map tag found");

	uint32_t entry_size = p->entry_size;
	uint32_t total_size = p->size;
	uintptr_t kern_end = (uintptr_t)&_kernel_end_phys;

	struct multiboot_mmap_entry *mmap;
	// Loop over memory map
	for (mmap = p->entries; (uint8_t *)mmap < (uint8_t *)p + total_size;
	     mmap = (struct multiboot_mmap_entry *)((uintptr_t)mmap + entry_size)) {
		// Insert a new region for each node in the memory map
		struct mmap_type *target;
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			// Here we impose some further restrictions.
			// Must be above or equal to where the kernel is mapped.
			// Try and squeeze any available memory into the mmap.
			if (mmap->addr + mmap->len < kern_end)
				target = &mem_map.reserved;
			else if (mmap->addr < kern_end) {
				mmap_insert_region(&mem_map.reserved, mmap->addr, kern_end - mmap->addr, MMAP_NONE);
				mmap_insert_region(&mem_map.available, kern_end, mmap->len - (kern_end - mmap->addr), MMAP_NONE);
				continue;
			} else
				target = &mem_map.available;
		} else
			target = &mem_map.reserved;
		mmap_insert_region(target, mmap->addr, mmap->len, MMAP_NONE);
	}

	mem_map.highest_mapped = KERNEL_PHYS_MAP_END;
	return &mem_map;
}
