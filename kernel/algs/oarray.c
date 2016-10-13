#include <algs/oarray.h>
#include <memory/kheap.h>
#include <klog.h>
#include <string.h>
#include <sys/cdefs.h>

bool oarray_stdlthan_pred(void * a, void *b) {
	return (a < b) ? 1 : 0;
}

oarray oarray_create(uint32_t max_size, lthan_predicate less_than) {
	oarray returner;
	returner.array = (void*)kmalloc(max_size * sizeof(void *));
	memset(returner.array, 0, max_size * sizeof(void *));
	returner.size = 0;
	returner.max_size = max_size;
	returner.less_than = less_than;
	return returner;
}

oarray oarray_place(void *addr, uint32_t max_size, lthan_predicate less_than) {
	oarray returner;
	returner.array = (void*)addr;
	memset(returner.array, 0, max_size * sizeof(void *));
	returner.size = 0;
	returner.max_size = max_size;
	returner.less_than = less_than;
	return returner;

}

void oarray_destroy(oarray *array) {
	kfree(array->array);
}
void oarray_insert(void *item, oarray *array) {
	klog_assert(array->less_than);
	uint32_t iter = 0;
	while (iter < array->size && array->less_than(array->array[iter], item))
		iter++;

	if (iter == array->size)
		array->array[array->size++] = item;
	else {
		void *tmp = array->array[iter];
		array->array[iter] = item;
		while (iter < array->size) {
			iter++;
			void *tmp2 = array->array[iter];
			array->array[iter] = tmp;
			tmp = tmp2;
		}
		array->size++;
	}
}

void *oarray_lookup(uint32_t i, oarray *array) {
	klog_assert(i < array->size);
	return array->array[i];
}

void oarray_remove(uint32_t i, oarray *array) {
	while (i < array->size) {
		array->array[i] = array->array[i + 1];
		i++;
	}
	array->size--;
}
