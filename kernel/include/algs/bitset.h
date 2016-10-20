#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct bitset {
	uint8_t *data;
	size_t size;
};

void bitset_create(struct bitset *, size_t size);
void bitset_free(struct bitset *);
void bitset_set(struct bitset *, size_t bit);
void bitset_clear(struct bitset *, size_t bit);
bool bitset_test(struct bitset *, size_t bit);
bool bitset_find_first(struct bitset *, size_t *out);
