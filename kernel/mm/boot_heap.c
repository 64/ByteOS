#include <stddef.h>

#include "mm.h"
#include "libk.h"

#define HEAP_MAGIC 0x9876543212345678
#define MIN_ALLOC 16

extern uintptr_t boot_heap_start;
extern uintptr_t boot_heap_end;

struct heap_hdr {
	size_t size;
	union {
		uint64_t magic;
		uintptr_t next;
	};
};

static struct heap_hdr *heap;

static struct heap_hdr *init_heap(void) {
	struct heap_hdr *rv = (struct heap_hdr *)&boot_heap_start;
	rv->size = (uintptr_t)&boot_heap_start - (uintptr_t)&boot_heap_end;
	rv->magic = HEAP_MAGIC;
	return rv;
}

void *boot_heap_malloc(size_t n) {
	if (heap == NULL)
		heap = init_heap();

	// Align n to 16 bytes
	n = ((n + 15) & -16);

	struct heap_hdr *prev = NULL;
	struct heap_hdr *head = heap;

	/*while (head != NULL) {

	}*/

	return NULL;
}

void boot_heap_free(void __attribute__((unused)) *p) {
	struct heap_hdr *prev = NULL;
	struct heap_hdr *head = heap;

	/*while (head != NULL) {

	}*/

	return NULL;
}
