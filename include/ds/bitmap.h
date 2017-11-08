#pragma once
#include <stdbool.h>
#include <stddef.h>

#define bitmap_test(map, which) (map[(which) >> 3] & (1 << ((which) & 7)))

#define bitmap_set(map, which) map[(which) >> 3] |= (1 << ((which) & 7))

#define bitmap_clear(map, which) map[(which) >> 3] &= ~(1 << ((which) & 7))

#define bitmap_toggle(map, which) map[(which) >> 3] ^= (1 << ((which) & 7))

//https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
#define bitmap_write(map, which, state) map[(which) >> 3] ^= (-(state) ^ map[(which) >> 3]) & (1 << ((which) & 7))

int bitmap_find_hole(unsigned char *map, size_t size, size_t hole_size);
