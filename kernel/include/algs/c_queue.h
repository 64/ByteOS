#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CQ_MAX 25

struct c_queue {
	int32_t head, tail;
	uint8_t data[CQ_MAX];
};

void c_queue_init(struct c_queue *);
void c_queue_clear(struct c_queue *);
bool c_queue_push(struct c_queue *, uint8_t data);
uint8_t c_queue_peek(struct c_queue *q);
uint8_t c_queue_pop(struct c_queue *);
bool c_queue_isempty(struct c_queue *);
bool c_queue_isfull(struct c_queue *);
