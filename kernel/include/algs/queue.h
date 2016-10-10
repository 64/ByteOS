#pragma once

#include <stdint.h>
#include <memory/kheap.h>

typedef struct {
	uint32_t *data;
	size_t front;
	size_t length;
	size_t capacity;
} queue_int;

queue_int *queue_int_create(size_t length);
size_t queue_int_length(queue_int *q);
uint32_t queue_int_peek(queue_int *q, bool *err);
uint32_t queue_int_push(queue_int *q, uint32_t data, bool *err);
uint32_t queue_int_pop(queue_int *q, bool *err);
bool queue_int_isfull(queue_int *q);
bool queue_int_isempty(queue_int *q);
void queue_int_free(queue_int *q);
