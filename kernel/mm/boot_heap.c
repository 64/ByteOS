#include <stddef.h>

#include "ds/bitmap.h"
#include "mm.h"
#include "util.h"
#include "libk.h"

// This must be the same size as the heap size in cpu/long_mode.asm
#define NBITS (1024 * 64 / 16)
#define MIN_NODE 4096

extern uintptr_t boot_heap_start;
extern uintptr_t boot_heap_end;
extern physaddr_t _kernel_end_phys;

static unsigned char bitmap[NBITS / 8];

// Stored at the start of each usable node of memory
struct heap_node {
	physaddr_t usable_start;
	size_t usable_len;
	struct heap_node *next;
	unsigned char bitmap[];
};

struct heap_node *heap_start;

void boot_heap_register_node(physaddr_t start_addr, size_t len) {
	if (start_addr + len > (physaddr_t)&_kernel_end_phys + MIN_NODE && len >= MIN_NODE)
		start_addr = MAX((physaddr_t)&_kernel_end_phys, start_addr);
	else
		return;

	struct heap_node *new_node = (struct heap_node *)(start_addr + KERNEL_TEXT_BASE);
	size_t bitmap_bits = len / 4096;
	size_t bitmap_bytes = ((bitmap_bits + 7) & -8) / 8; // Align up to eight
	kassert(bitmap_bytes < len);
	new_node->usable_len = len - bitmap_bytes;
	new_node->usable_start = start_addr + (sizeof(struct heap_node) + bitmap_bytes);
	new_node->next = heap_start;
	heap_start = new_node;
	kprintf("Memory node: %p, %zu\n", (void *)new_node->usable_start, new_node->usable_len);

	// Mark first pages as used
	size_t pages_used = ALIGNUP(bitmap_bytes + sizeof(struct heap_node), 4096) / 4096;
	for (size_t idx = 0; idx < pages_used; idx++)
		bitmap_set(new_node->bitmap, idx);
	kprintf("Size used: %zu (%zu pages)\n", bitmap_bytes, pages_used);
}

void *boot_heap_malloc(size_t n) {
	n = ALIGNUP(n, 16);

	int idx = bitmap_find_hole(bitmap, NBITS, n / 16);
	if (idx == -1)
		// No more blocks
		panic("boot heap can't satisfy allocation of size %zu", n);

	// Mark as used
	size_t n_bits = n / 16;
	uintptr_t p = (idx * 16) + (uintptr_t)&boot_heap_start;
	while (n_bits-- != 0)
		bitmap_set(bitmap, idx), idx++;
	return (void *)p;
}

void boot_heap_free(void  *p, size_t n) {
	n = ALIGNUP(n, 16);
	size_t n_bits = n / 16;
	size_t bit_idx = ((uintptr_t)p - (uintptr_t)&boot_heap_start) / 16;
	// Mark as unused
	while (n_bits-- != 0)
		bitmap_clear(bitmap, bit_idx), bit_idx++;
}
