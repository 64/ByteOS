#include <algs/c_queue.h>
#include <klog.h>
#include <string.h>

/// \brief Creates and initialises a new byte queue
/// \param q A pointer to where the new queue is to be initialised
void c_queue_init(struct c_queue *q) {
	q->head = q->tail = -1;
	memset(q->data, 0, sizeof(q->data));
}

/// \brief Clears all the data in a queue
/// \param q The queue to be cleared
void c_queue_clear(struct c_queue *q) {
	q->head = q->tail = -1;
	memset(q->data, 0, sizeof(q->data));
}

/// \brief Pushes a byte onto the back of a queue
/// \param q The queue to be pushed onto
/// \param data The byte to append
/// \return One if successful, otherwise zero
bool c_queue_push(struct c_queue *q, uint8_t data) {
	if (c_queue_isfull(q)) {
		klog_warn("Queue full on push!\n");
		return 0;
	}
	q->tail++;
	q->data[q->tail % CQ_MAX] = data;
	return 1;
}

/// \brief Pops a byte from the front of a queue
/// \param q The queue to be popped from
/// \return The newly popped byte
uint8_t c_queue_pop(struct c_queue *q) {
	if (c_queue_isempty(q)) {
		klog_warn("Queue empty on pop!\n");
		return 0;
	}
	q->head++;
	return q->data[q->head % CQ_MAX];
}

/// \brief Peeks at the value at the front of a queue
/// \param q The queue to be peeked
/// \return The byte on the front of the queue
uint8_t c_queue_peek(struct c_queue *q) {
	if (c_queue_isempty(q)) {
		klog_warn("Queue empty on peek!\n");
		return 0;
	}
	return q->data[(q->head + 1) % CQ_MAX];
}

/// \brief Checks if a queue is empty
/// \param q The queue to be checked
/// \return One if the queue is empty, otherwise zero
bool c_queue_isempty(struct c_queue *q) {
	return (q->head == q->tail);
}

/// \brief Checks if a queue is full
/// \param q The queue to be checked
/// \return One if the queue is full, otherwise zero
bool c_queue_isfull(struct c_queue *q) {
	return ((q->tail - CQ_MAX) == q->head);
}
