#include <stddef.h>

#include "mm.h"
#include "libk.h"

#define HEAP_MAGIC 0x9876543212345678
#define HEAP_INVALID (struct heap_hdr *)0xFFFFFFFFFFFFFFFF
#define MIN_ALLOC 16
#define SIZE sizeof(struct heap_hdr)

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
	rv->size = (uintptr_t)&boot_heap_end - (uintptr_t)&boot_heap_start;
	rv->next = 0;
	return rv;
}

static size_t dist_to_next(struct heap_hdr *ptr) {
	if (ptr->magic == HEAP_MAGIC)
		return ptr->size + SIZE;
	return ptr->next == 0 ? 0 : ptr->size + SIZE + ptr->next;
}

static inline void do_alloc(size_t n, struct heap_hdr *head, struct heap_hdr *prev) {
	kprintf("Head size %zu\n", head->size);
	if (n + SIZE + MIN_ALLOC <= head->size) {
		// Room for new node
		struct heap_hdr *new_node = (struct heap_hdr *)(SIZE + n + (uintptr_t)head);
		new_node->size = head->size - SIZE - n;
		if (head->magic == HEAP_MAGIC)
			new_node->magic = HEAP_MAGIC;
		else if (head->next == 0)
			new_node->next = 0;
		else
			new_node->next = head->next - n;
		head->size = n;
		head->magic = HEAP_MAGIC;
		kprintf("New node at %p, size %zu, next: %lu\n", (void *)new_node, new_node->size, new_node->next);
	} else {
		// No room for new node
		if (head->magic != HEAP_MAGIC)
			head->next = dist_to_next(head);
	}

	// Update previous
	if (prev != NULL) {
		if (prev->magic == HEAP_MAGIC) {
			prev->next = dist_to_next(head);
		} else {
			prev->next += dist_to_next(head);
		}
	} else if (dist_to_next(head) == 0)
		heap = HEAP_INVALID;
	else
		heap = (struct heap_hdr *)((uintptr_t)head + dist_to_next(head));
}

void print_heap(void) {
	struct heap_hdr *head = heap;

	kprintf("Dumping heap...\n");
	if (head == HEAP_INVALID) {
		kprintf("No free space!\n");
		return;
	}

	do {
		kprintf("Node: %p, length %zu\n", (void *)head, head->size);
		struct heap_hdr *next_head = (struct heap_hdr *)((uintptr_t)head + dist_to_next(head));
		if (next_head == head)
			break;
		head = next_head;
	} while (head->next != 0);

	kprintf("Finished heap dump.\n");
}

void *boot_heap_malloc(size_t n) {
	if (heap == NULL)
		heap = init_heap();
	else if (heap == HEAP_INVALID)
		panic("boot heap out of memory");

	// Align n to 16 bytes
	n = ((n + 15) & -16);

	struct heap_hdr *prev = NULL;
	struct heap_hdr *head = heap;

	while (1) {
		if (head->size >= n) {
			do_alloc(n, head, prev);
			return head;
		} else if (head->magic == HEAP_MAGIC) {
			// Beginning of a block
			prev = head;
			uintptr_t next_addr = (uintptr_t)head + head->size + SIZE;
			head = (struct heap_hdr *)next_addr;
		} else if (head->next != 0) {
			// End of a block
			prev = head;
			uintptr_t next_addr = (uintptr_t)head + head->size + head->next + SIZE;
			head = (struct heap_hdr *)next_addr;
		} else {
			// No more blocks
			print_heap();
			panic("boot heap can't satisfy allocation of size %zu", n);
		}
	}

	__builtin_unreachable();
}

void boot_heap_free(void __attribute__((unused)) *p) {
	//struct heap_hdr *prev = NULL;
	//struct heap_hdr *head = heap;

	/*while (head != NULL) {

	}*/

	return;
}
