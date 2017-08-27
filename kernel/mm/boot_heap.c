#include <stddef.h>

#include "ds/bitmap.h"
#include "mm.h"
#include "libk.h"

// This must be the same size as the heap size in cpu/long_mode.asm
#define NBITS (1024 * 64 / 16)

extern uintptr_t boot_heap_start;
extern uintptr_t boot_heap_end;

static unsigned char bitmap[NBITS / 8];

void *boot_heap_malloc(size_t n) {
	// Align n to 16 bytes
	n = ((n + 15) & -16);

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
	// Align n to 16 bytes
	n = ((n + 15) & -16);
	size_t n_bits = n / 16 * 8;
	size_t bit_idx = ((uintptr_t)p - (uintptr_t)&boot_heap_start) / 16 * 8;
	// Mark as unused
	while (n_bits-- != 0)
		bitmap_clear(bitmap, bit_idx), bit_idx++;
}
