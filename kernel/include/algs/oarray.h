#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef bool (*lthan_predicate)(void *, void*);

typedef struct {
	void **array;
	uint32_t size;
	uint32_t max_size;
	lthan_predicate less_than;
} oarray;

bool oarray_stdlthan_pred(void * a, void *b);
oarray oarray_create(uint32_t max_size, lthan_predicate less_than);
oarray oarray_place(void *addr, uint32_t max_size, lthan_predicate less_than);
void oarray_destroy(oarray *array);
void oarray_insert(void *item, oarray *array);
void *oarray_lookup(uint32_t i, oarray *array);
void oarray_remove(uint32_t i, oarray *array);
