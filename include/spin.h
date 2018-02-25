#pragma once

#include <stdbool.h>

#include "asm.h"

typedef uint64_t spinlock_t;

void spin_lock(spinlock_t * volatile lock);
void spin_unlock(spinlock_t * volatile lock);

#define spin_lock_irqsave(lock, rflags) ({ \
	rflags = read_rflags(); \
	cli(); \
	spin_lock(lock); })

#define spin_unlock_irqsave(lock, rflags) ({ \
	spin_unlock(lock); \
	write_rflags(rflags); })

static inline void spin_init(spinlock_t * volatile lock)
{
	*lock = 0;
}
