#pragma once

#include "spin.h"

// TODO: Add sleeping

typedef struct {
	spinlock_t lock;
} mutex_t;

static inline void mutex_lock(mutex_t *mutex)
{
	spin_lock(&mutex->lock);
}

static inline void mutex_unlock(mutex_t *mutex)
{
	spin_unlock(&mutex->lock);
}
