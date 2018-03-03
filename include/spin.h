#pragma once

#include <stdbool.h>

#include "asm.h"

typedef uint64_t spinlock_t;

void spin_lock(volatile spinlock_t *lock);
void spin_unlock(volatile spinlock_t *lock);

#define spin_lock_irqsave(lock, rflags) ({ \
	rflags = read_rflags(); \
	cli(); \
	spin_lock(lock); })

#define spin_unlock_irqsave(lock, rflags) ({ \
	spin_unlock(lock); \
	write_rflags(rflags); })

static inline void spin_init(volatile spinlock_t *lock)
{
	*lock = 0;
}
