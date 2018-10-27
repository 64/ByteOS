#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "mm_types.h"
#include "smp.h"
#include "util.h"

struct percpu {
	// Be careful changing these four variables as they are referenced in asm
	struct task *current_task; // Current task
	virtaddr_t rsp_scratch; // Temporary rsp used for syscalls
	virtaddr_t tss; // Holds the address of the TSS (needed for context switches)
	uint64_t preempt_count; // Incremented for every lock held and decremented for every lock released

	atomic32_t int_depth;
	bool reschedule;

	cpuid_t id;
	uint8_t apic_id;
	struct runq *run_queue;
};

void percpu_init_ap(void);
void percpu_init_bsp(void);
void percpu_set_addr(struct percpu *);

#define __percpu(var) (((struct percpu *)NULL)->var)
#define __percpu_type(var) typeof(__percpu(var))
#define __percpu_marker(var)	((volatile __percpu_type(var) *)&__percpu(var))

#define percpu_get(var) ({						\
	__percpu_type(var) res;						\
	asm ("mov %%gs:%1, %0"  					\
	     : "=r" (res)						\
	     : "m" (*__percpu_marker(var))                              \
	);                                                              \
	res; })

#define __percpu_set(suffix, var, val)					\
({									\
	asm ("mov" suffix " %1, %%gs:%0"				\
	     : "=m" (*__percpu_marker(var))				\
	     : "ir" (val));						\
})

#define percpu_set(var, val)						\
({									\
	switch (sizeof(__percpu_type(var))) {				\
	case 1: __percpu_set("b", var, val); break;			\
	case 2: __percpu_set("w", var, val); break;			\
	case 4: __percpu_set("l", var, val); break;			\
	case 8: __percpu_set("q", var, val); break;			\
	default: _static_assert(false);				        \
	}								\
}) 

#define current (percpu_get(current_task))

extern struct percpu *percpu_table[];

static inline struct percpu *percpu_get_id(cpuid_t id)
{
	return percpu_table[id];
}

static inline struct percpu *percpu_self(void)
{
	return percpu_table[percpu_get(id)];
}

