#pragma once

#include <stdint.h>

#include "mm.h"

#define TASK_KTHREAD (1 << 0)
#define TASK_NONE 0

struct task {
	virtaddr_t rsp_top;
	struct mmu_info *mmu;
	uint64_t flags;
	virtaddr_t rsp_original;
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

struct task *task_fork(struct task *parent, virtaddr_t entry, uint64_t flags, const struct callee_regs *regs);
void task_execve(virtaddr_t function, char *argv[], unsigned int flags);
