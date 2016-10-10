#include <algs/queue.h>
#include <string.h>

#define Q_INDEX_OFFSET(front, len, max) q->data[((front) + (len)) % (max)]

queue_int *queue_int_create(size_t length) {
	queue_int *ret = (queue_int *)kmalloc(sizeof(queue_int));
	ret->data = (uint32_t *)kmalloc(sizeof(uint32_t) * length);
	memset(ret->data, 0, sizeof(uint32_t) * length);
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

uint32_t queue_int_push(queue_int *q, uint32_t data, bool *err) {
	if (queue_int_isfull(q)) {
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
		uint32_t rv = q->data[q->front];
		if (q->length != 0)
			q->front = (q->front == (q->capacity - 1)) ? 0 : q->front + 1;
		return rv;
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
