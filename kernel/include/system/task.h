#pragma once

#include <stdint.h>
#include <system.h>

struct task {
	struct registers regs;
	struct task *next;
};

void tasking_init(void);
void task_create(struct task *, void(*)(), uint32_t, uint32_t*);
void task_preempt();

// Defined in system/task_switch.S
extern void task_switch(struct registers *previous, struct registers *current);
