#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef bool (*lthan_predicate)(void *, void*);

struct oarray {
	void **array;
	uint32_t size;
	uint32_t max_size;
	lthan_predicate less_than;
};

bool oarray_stdlthan_pred(void * a, void *b);
struct oarray oarray_create(uint32_t max_size, lthan_predicate less_than);
struct oarray oarray_place(void *addr, uint32_t max_size, lthan_predicate less_than);
void oarray_destroy(struct oarray *array);
void oarray_insert(void *item, struct oarray *array);
void *oarray_lookup(uint32_t i, struct oarray *array);
void oarray_remove(uint32_t i, struct oarray *array);
