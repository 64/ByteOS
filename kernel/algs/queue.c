#include <algs/queue.h>

#define Q_INDEX_OFFSET(front, len, max) q->data[((front) + (len)) % (max)]

queue_int *queue_int_create(size_t length) {
	queue_int *ret = (queue_int *)kmalloc(sizeof(queue_int));
	ret->data = (uint32_t *)kmalloc(sizeof(uint32_t) * length);
	ret->length = 0;
	ret->front = 0;
	ret->capacity = length ? length : 1; // Prevent subtle errors
	return ret;
}

size_t queue_int_length(queue_int *q) {
	return q->length;
}

uint32_t queue_int_peek(queue_int *q, bool *err) {
	if (queue_int_isempty(q)) {
		*err = 1;
		return 0;
	} else {
		return q->data[q->front];
	}
}

uint32_t queue_int_push(uint32_t data, queue_int *q, bool *err) {
	if (queue_int_isfull(q) || q->length == (q->capacity - 1)) {
		*err = 1;
		return data;
	} else {
		q->data[(q->front + q->length) % q->capacity] = data;
		q->length++;
		return data;
	}
}

uint32_t queue_int_pop(queue_int *q, bool *err) {
	if (queue_int_isempty(q)) {
		*err = 1;
		return 0;
	} else {
		q->length--;
		q->front = (q->front == 0) ? q->capacity - 1 : q->front - 1;
		return q->data[q->front];
	}
}

bool queue_int_isfull(queue_int *q) {
	return q->length == q->capacity;
}

bool queue_int_isempty(queue_int *q) {
	return q->length == 0;
}

void queue_int_free(queue_int *q) {
	if (q != NULL)
		kfree(q->data);
	kfree(q);
}
