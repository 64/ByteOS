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
	struct slist_entry next_node;
	unsigned char bitmap[];
};

struct heap_node *heap_start;

static inline size_t calc_bitmap_size(struct heap_node *node) {
	size_t header_size = node->usable_start - ((physaddr_t)node - KERNEL_TEXT_BASE);
	size_t full_size = header_size + node->usable_len;
	size_t bitmap_bits = full_size / BLOCKSIZE;
	return bitmap_bits;
}

void boot_heap_register_node(physaddr_t start_addr, size_t len) {
	if (start_addr + len > (physaddr_t)&_kernel_end_phys + MIN_NODE && len >= MIN_NODE)
		start_addr = MAX((physaddr_t)&_kernel_end_phys, start_addr);
	else
		return;

	struct heap_node *new_node = (struct heap_node *)(start_addr + KERNEL_TEXT_BASE);
	size_t bitmap_bits = len / BLOCKSIZE;
	kprintf("Bitmap bits: %zu\n", bitmap_bits);
	size_t bitmap_bytes = ALIGNUP(bitmap_bits, 8) / 8; // Align up to eight
	kassert(bitmap_bytes < len);
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

void *boot_heap_malloc(size_t n) {
	n = ALIGNUP(n, BLOCKSIZE);

	slist_foreach(current, next_node, heap_start) {
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

void boot_heap_free(void  *p, size_t n) {
	n = ALIGNUP(n, BLOCKSIZE);

	slist_foreach(current, next_node, heap_start) {
		// TODO: Use translation macro
		if (current->usable_start + KERNEL_TEXT_BASE > (uintptr_t)p
			|| current->usable_start + KERNEL_TEXT_BASE + current->usable_len <= (uintptr_t)p)
			continue;

		size_t n_bits = n / BLOCKSIZE;
		size_t bit_idx = ((uintptr_t)p - (uintptr_t)current) / BLOCKSIZE;
		// Mark as unused
		while (n_bits-- != 0)
			bitmap_clear(current->bitmap, bit_idx), bit_idx++;
	}
}
