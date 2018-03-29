#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "types.h"
#include "smp.h"
#include "util.h"

struct percpu {
	// Be careful changing these three variables as they are referenced in asm
	struct task *current_task;
	virtaddr_t rsp_scratch;
	virtaddr_t tss;
	uint32_t preempt_count;

	cpuid_t id;
	struct runq *run_queue;
};

void percpu_init_ap(void);
void percpu_init_bsp(void);
void percpu_set_addr(struct percpu *);

extern struct percpu *percpu_table[];
static inline struct percpu *percpu_get_id(cpuid_t id)
{
	return percpu_table[id];
}

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
