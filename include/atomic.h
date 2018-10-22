#pragma once

#include <stdint.h>

#include "asm.h"

typedef volatile struct {
	volatile uint32_t var;
} atomic32_t;

typedef volatile struct {
	volatile uint64_t var;
} atomic64_t;

typedef atomic32_t kref_t;

static inline uint32_t atomic_read32(atomic32_t *a)
{
	return __atomic_load_n(&a->var, __ATOMIC_SEQ_CST);
}

static inline uint32_t atomic_inc_read32(atomic32_t *a)
{
	return __atomic_add_fetch(&a->var, 1, __ATOMIC_SEQ_CST);
}

static inline uint32_t atomic_dec_read32(atomic32_t *a)
{
	return __atomic_sub_fetch(&a->var, 1, __ATOMIC_SEQ_CST);
}

static inline void atomic_write32(atomic32_t *a, uint32_t val)
{
	__atomic_store_n(&a->var, val, __ATOMIC_SEQ_CST);
}


static inline uint64_t atomic_read64(atomic64_t *a)
{
	return __atomic_load_n(&a->var, __ATOMIC_SEQ_CST);
}

static inline uint64_t atomic_inc_read64(atomic64_t *a)
{
	return __atomic_add_fetch(&a->var, 1, __ATOMIC_SEQ_CST);
}

static inline uint64_t atomic_dec_read64(atomic64_t *a)
{
	return __atomic_sub_fetch(&a->var, 1, __ATOMIC_SEQ_CST);
}

static inline void atomic_write64(atomic64_t *a, uint32_t val)
{
	__atomic_store_n(&a->var, val, __ATOMIC_SEQ_CST);
}

static inline void kref_inc(kref_t *k)
{
	atomic_inc_read32(k);
}

static inline void kref_dec(kref_t *k)
{
	atomic_dec_read32(k);
}

static inline uint32_t kref_inc_read(kref_t *k)
{
	return atomic_inc_read32(k);
}

static inline uint32_t kref_dec_read(kref_t *k)
{
	return atomic_dec_read32(k);
}

static inline uint32_t kref_read(kref_t *k)
{
	return atomic_read32(k);
}
