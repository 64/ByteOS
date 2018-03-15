#pragma once

#include <stdint.h>

#include "mm.h"

#define TASK_KTHREAD (1 << 0)
#define TASK_NEED_PREEMPT (1 << 1)
#define TASK_NONE 0

struct task {
	// Careful not to move these as they are referenced in asm
	virtaddr_t rsp_top;
	virtaddr_t rsp_original;
	struct mmu_info *mmu;

	// Scheduler information
	struct dlist_entry list;

	// Process state
	enum {
		TASK_RUNNABLE,
		TASK_RUNNING,
		TASK_BLOCKED
	} state;
	uint64_t flags; // Includes TASK_NEED_PREEMPT flag
	uint64_t preempt_count; // Number of locks held (preemption disabled when > 0)
};

struct callee_regs {
	uint64_t rsp;
	uint64_t rbp;
	uint64_t rbx;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
};

void switch_to(struct task *);

void schedule(void);
void sched_run(void);
void sched_add(struct task *t);

struct task *task_fork(struct task *parent, virtaddr_t entry, uint64_t flags, const struct callee_regs *regs);
void task_execve(virtaddr_t function, char *argv[], unsigned int flags);
