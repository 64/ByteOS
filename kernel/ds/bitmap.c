#include "ds/bitmap.h"

// TODO: Optimize this (lookup tables, etc)
int bitmap_find_hole(unsigned char *map, size_t size, size_t hole_size) {
	size_t i, temp = 0;
	for (i = 0; i < size; i++) {
		if (temp == hole_size)
			return i - temp;
		else if (bitmap_test(map, i) == 0)
			temp++;
		else
			temp = 0;
	}
	return -1;
}
