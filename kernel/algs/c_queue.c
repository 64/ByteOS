#include <algs/c_queue.h>
#include <klog.h>
#include <string.h>

void c_queue_init(struct c_queue *q) {
	q->head = q->tail = -1;
	memset(q->data, 0, sizeof(q->data));
}

void c_queue_clear(struct c_queue *q) {
	q->head = q->tail = -1;
	memset(q->data, 0, sizeof(q->data));
}

bool c_queue_push(struct c_queue *q, uint8_t data) {
	if (c_queue_isfull(q)) {
		klog_warn("Queue full on push!\n");
		return 0;
	}
	q->tail++;
	q->data[q->tail % CQ_MAX] = data;
	return 1;
}

uint8_t c_queue_pop(struct c_queue *q) {
	if (c_queue_isempty(q)) {
		klog_warn("Queue empty on pop!\n");
		return 0;
	}
	q->head++;
	return q->data[q->head % CQ_MAX];
}

uint8_t c_queue_peek(struct c_queue *q) {
	if (c_queue_isempty(q)) {
		klog_warn("Queue empty on peek!\n");
		return 0;
	}
	return q->data[(q->head + 1) % CQ_MAX];
}

bool c_queue_isempty(struct c_queue *q) {
	return (q->head == q->tail);
}

bool c_queue_isfull(struct c_queue *q) {
	return ((q->tail - CQ_MAX) == q->head);
}
