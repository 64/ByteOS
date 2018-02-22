#pragma once

typedef int spinlock_t;

static inline void spin_lock(spinlock_t *lock)
{
	while (!__sync_bool_compare_and_swap(lock, 0, 1))
		;
	__sync_synchronize();
}

static inline void spin_unlock(spinlock_t *lock)
{
	__sync_synchronize();
	*lock = 0;
}

