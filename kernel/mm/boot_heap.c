#include <stddef.h>

#include "ds/bitmap.h"
#include "ds/linked.h"
#include "mm.h"
#include "util.h"
#include "libk.h"

#define BLOCKSIZE 4096
#define MIN_NODE BLOCKSIZE

extern uintptr_t boot_heap_start;
extern uintptr_t boot_heap_end;
extern physaddr_t _kernel_end_phys;

// Stored at the start of each usable node of memory
struct heap_node {
	physaddr_t usable_start;
	size_t usable_len;
	uint64_t mem_flags;
	struct slist_entry next_node;
	unsigned char bitmap[];
};

struct heap_node *heap_start;

static inline size_t calc_bitmap_size(struct heap_node *node) {
	size_t header_size = node->usable_start - kern_to_phys(node);
	size_t full_size = header_size + node->usable_len;
	size_t bitmap_bits = full_size / BLOCKSIZE;
	return bitmap_bits;
}

void boot_heap_register_node(physaddr_t start_addr, size_t len, uint64_t flags) {
	const physaddr_t kern_end = (physaddr_t)&_kernel_end_phys;

	if (start_addr + len > kern_end + MIN_NODE && len >= MIN_NODE)
		start_addr = MAX(kern_end, start_addr);
	else
		return;

	if (start_addr < kern_end) {
		if (start_addr + len >= kern_end)
			boot_heap_register_node(kern_end, start_addr + len - kern_end, MEM_NORMAL);
		return;
	}

	struct heap_node *new_node = (struct heap_node *)phys_to_kern(start_addr);
	size_t bitmap_bits = len / BLOCKSIZE;
	kprintf("Bitmap bits: %zu\n", bitmap_bits);
	size_t bitmap_bytes = ALIGNUP(bitmap_bits, 8) / 8; // Align up to eight
	kassert(bitmap_bytes < len);
	new_node->mem_flags = flags;
	new_node->usable_len = len - bitmap_bytes;
	new_node->usable_start = start_addr + (sizeof(struct heap_node) + bitmap_bytes);
	slist_set_next(new_node, next_node, heap_start);
	heap_start = new_node;
	kprintf("Memory node: %p, %zu\n", (void *)new_node->usable_start, new_node->usable_len);

	// Mark first pages as used
	size_t pages_used = ALIGNUP(bitmap_bytes + sizeof(struct heap_node), BLOCKSIZE) / BLOCKSIZE;
	for (size_t idx = 0; idx < pages_used; idx++)
		bitmap_set(new_node->bitmap, idx);
	// Mark padding bits at the end as used
	size_t padding_bits = (bitmap_bytes * 8) - bitmap_bits;
	for (size_t idx = 0; idx < padding_bits; idx++)
		bitmap_set(new_node->bitmap, idx + bitmap_bits);
}

void boot_heap_map_header_addr(virtaddr_t)

static void *_boot_heap_alloc(size_t n, uint64_t alloc_flags) {
	n = ALIGNUP(n, BLOCKSIZE);

	slist_foreach(current, next_node, heap_start) {
		// Ignore if it doesn't satisfy our permanent mapping request
		if (alloc_flags == MEM_ALWAYS_MAPPED
			&& current->mem_flags != MEM_ALWAYS_MAPPED)
			continue;

		int idx = bitmap_find_hole(current->bitmap, calc_bitmap_size(current), n / BLOCKSIZE);
		if (idx == -1)
			// No more blocks
			continue;

		// Mark as used
		size_t n_bits = n / BLOCKSIZE;
		uintptr_t p = (idx * BLOCKSIZE) + (uintptr_t)current;
		while (n_bits-- != 0)
			bitmap_set(current->bitmap, idx), idx++;
		return (void *)p;
	}

	panic("boot heap can't satisfy allocation of size %zu", n);
}

void *boot_heap_malloc(size_t n) {
	return _boot_heap_alloc(n, MEM_NORMAL);
}

void *boot_heap_malloc_mapped(size_t n) {
	return _boot_heap_alloc(n, MEM_ALWAYS_MAPPED);
}

void boot_heap_free(void  *p, size_t n) {
	n = ALIGNUP(n, BLOCKSIZE);

	slist_foreach(current, next_node, heap_start) {
		if (phys_to_kern(current->usable_start) > p
			|| phys_to_kern(current->usable_start + current->usable_len) <= p)
			continue;

		size_t n_bits = n / BLOCKSIZE;
		size_t bit_idx = ((uintptr_t)p - (uintptr_t)current) / BLOCKSIZE;
		// Mark as unused
		while (n_bits-- != 0)
			bitmap_clear(current->bitmap, bit_idx), bit_idx++;
	}
}
