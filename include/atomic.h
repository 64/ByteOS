#pragma once

#include <stdint.h>

typedef struct {
	volatile uint64_t counter;
} atomic_t;

static inline uint64_t atomic_read(atomic_t *a)
{
	return __atomic_load_n(&a->counter, __ATOMIC_SEQ_CST);
}

static inline uint64_t atomic_inc_load(atomic_t *a)
{
	return __atomic_add_fetch(&a->counter, 1, __ATOMIC_SEQ_CST);
}

static inline uint64_t atomic_dec_load(atomic_t *a)
{
	return __atomic_sub_fetch(&a->counter, 1, __ATOMIC_SEQ_CST);
}


