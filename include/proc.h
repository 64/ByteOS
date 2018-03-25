#pragma once

#include <stdint.h>

#include "mm.h"
#include "smp.h"

#define TASK_KTHREAD (1 << 0)
#define TASK_NEED_PREEMPT (1 << 1)
#define TASK_NONE 0

typedef int32_t pid_t;

struct task {
	// Careful not to move these as they are referenced in asm
	virtaddr_t rsp_top;
	virtaddr_t rsp_original;
	struct mmu_info *mmu;

	// Scheduler information
	struct dlist_entry list;

	// Process state
	enum task_state {
		TASK_RUNNABLE,
		TASK_RUNNING,
		TASK_NOT_STARTED,
		TASK_BLOCKED
	} state;

	uint64_t flags; // Includes TASK_NEED_PREEMPT flag
	pid_t pid;

	cpuset_t affinity; // Defines which processors this task can run on
};

struct runq {
	spinlock_t lock;
	struct task *head, *idle;
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
void sched_yield(void);
void sched_add(struct task *t);

struct task *task_fork(struct task *parent, virtaddr_t entry, uint64_t flags, const struct callee_regs *regs);
void task_execve(virtaddr_t function, char *argv[], unsigned int flags);
void task_wakeup(struct task *t);
void task_exit(struct task *t, int code);

void runq_init(void);
void runq_start_balancer(void);
void runq_add(struct task *t);
void runq_remove(struct task *t);
struct task *runq_next(void);

void idle_task(void);
