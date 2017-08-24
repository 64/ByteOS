#include <stddef.h>

#include "mm.h"
#include "libk.h"

#define MIN_ALLOC 16
#define SIZE sizeof(struct heap_hdr)

extern uintptr_t boot_heap_start;
extern uintptr_t boot_heap_end;

struct heap_hdr {
	size_t size;
	struct heap_hdr *next;
};

static struct heap_hdr *heap;

void boot_heap_init(void) {
	heap = (struct heap_hdr *)&boot_heap_start;
	heap->size = (uintptr_t)&boot_heap_end - (uintptr_t)&boot_heap_start;
	heap->next = NULL;
}

static inline struct heap_hdr *get_next(struct heap_hdr *ptr) {
	if (ptr == NULL || ptr->next == NULL)
		return NULL;
	else
		return ptr->next;
}

static inline void do_alloc(size_t n, struct heap_hdr *head, struct heap_hdr *prev) {
	if (n + SIZE + MIN_ALLOC <= head->size) {
		// Room for new node
		struct heap_hdr *new_node = (struct heap_hdr *)(SIZE + n + (uintptr_t)head);
		new_node->size = head->size - SIZE - n;
		new_node->next = head->next;
		head->size = n;
		head->next = new_node;
	}

	if (prev != NULL)
		prev->next = head->next;
	else
		heap = head->next;
}

void boot_heap_print(void) {
	struct heap_hdr *head = heap;
	while (head != NULL) {
		kprintf("Node: %p, length: %zu\n", (void*)(uintptr_t)head, head->size);
		head = head->next;
	}
}

void *boot_heap_malloc(size_t n) {
	if (heap == NULL)
		goto fail;

	// Align n to 16 bytes
	n = ((n + 15) & -16);

	struct heap_hdr *prev = NULL;
	struct heap_hdr *head = heap;

 	do {
		if (head->size >= n) {
			do_alloc(n, head, prev);
			return (char *)head + SIZE;
		} else {
			prev = head;
			head = head->next;
		}
	} while (head->next != NULL);

fail:
	// No more blocks
	panic("boot heap can't satisfy allocation of size %zu", n);
}

void boot_heap_free(void  *p) {
	struct heap_hdr *prev_one = NULL;
	struct heap_hdr *prev_two = NULL;
	struct heap_hdr *head = heap;
	struct heap_hdr *target = (struct heap_hdr *)((uintptr_t)p - SIZE);

	if (head == NULL) {
		heap = target;
		target->next = NULL;
		return;
	} else if (target < head) {
		// Can we merge to the right?
		if ((uintptr_t)target + SIZE + target->size == (uintptr_t)head) {
			target->size += SIZE + head->size;
			target->next = head->next;
		} else {
			target->next = head;
		}
		heap = target;
		return;
	}

	for (; head != NULL; head = head->next) {
		if (head > target) {
			uintptr_t left_end = (uintptr_t)prev_one + SIZE + prev_one->size;
			uintptr_t target_end = (uintptr_t)target + SIZE + target->size;
			struct heap_hdr *ptr_to_update = prev_one;

			// Merge to the left
			if (prev_one != NULL && left_end == (uintptr_t)target) {
				prev_one->size += target->size + SIZE;
				prev_one->next = target->next;
				target = prev_one;
				ptr_to_update = prev_two;
			}

			// Merge to the right
			if (target_end == (uintptr_t)head) {
				target->size += head->size + SIZE;
				target->next = head->next;
			}

			if (ptr_to_update != NULL)
				ptr_to_update->next = target;
			return;
		} else if (head == target) {
			panic("boot heap double free at address %p", p);
		}
		prev_two = prev_one;
		prev_one = head;
	}
}
