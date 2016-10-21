#include <algs/bitset.h>
#include <klog.h>
#include <stdlib.h>
#include <memory/kheap.h>
#include <string.h>

void bitset_create(struct bitset *b, size_t size) {
	if (size == 0) {
		klog_warn("Bitset initialized with 0 size\n");
		return;
	}
	b->data = (uint8_t *)kmalloc(size);
	memset(b->data, 0, size);
	b->size = size;
}

void bitset_free(struct bitset *b) {
	kfree(b->data);
	memset(b->data, 0, b->size);
	b->size = 0;
}

void bitset_set(struct bitset *b, size_t bit) {
	if (b->size == 0) {
		klog_warn("Bitset: Bitset with size 0 was used\n");
		return;
	} else if ((b->size * 8) <= bit) {
		klog_warn("Bitset: Tried to access bit out of range\n");
		return;
	}
	size_t index = bit >> 3;
	bit = bit - index * 8;
	size_t offset = bit & 7;
	size_t mask = 1 << offset;
	b->data[index] |= mask;
}
void bitset_clear(struct bitset *b, size_t bit) {
	if (b->size == 0) {
		klog_warn("Bitset with size 0 was used\n");
		return;
	} else if ((b->size * 8) <= bit) {
		klog_warn("Bitset: Tried to access bit out of range\n");
		return;
	}
	size_t index = bit >> 3;
	bit = bit - index * 8;
	size_t offset = bit & 7;
	size_t mask = 1 << offset;
	b->data[index] &= ~mask;
}
bool bitset_test(struct bitset *b, size_t bit) {
	if ((b->size) == 0) {
		klog_fatal("Bitset with size 0 was used\n");
		abort();
	} else if ((b->size * 8) <= bit) {
		klog_warn("Bitset: Tried to access bit out of range\n");
		return 0;
	}
	size_t index = bit >> 3;
	bit = bit - index * 8;
	size_t offset = bit & 7;
	size_t mask = 1 << offset;
	return (b->data[index] & mask) != 0;
}

bool bitset_find_first(struct bitset *b, size_t *out) {
	size_t i, j;
	for (i = 0; i < b->size; i++) {
		if (b->data[i] != 0xFF)
			for (j = 0; j < 8; j++)
				if (bitset_test(b, (i * 8) + j) == 0) {
					*out = (i * 8) + j;
					return 1;
				}
	}
	return 0;
}

bool bitset_find_hole(struct bitset *b, size_t hole_size, size_t *out) {
	if (hole_size > (b->size * 8))
		return 0;

	size_t i, temp = 0;
	for (i = 0; i < b->size * 8; i++) {
		if (temp == hole_size) {
			*out = i - temp;
			return 1;
		} else if (bitset_test(b, i) == 0)
			temp++;
		else
			temp = 0;
	}
	return 0;
}
