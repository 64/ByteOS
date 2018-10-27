#pragma once

#include <stdint.h>

#include "mm.h"
#include "smp.h"
#include "fs.h"
#include "ds/rbtree.h"

#define TASK_KTHREAD (1 << 0)
#define TASK_RUNNING (1 << 1)
#define TASK_PREEMPTED (1 << 2)
#define TASK_NONE 0

#define FORK_KTHREAD (1 << 0)
#define FORK_UTHREAD (1 << 1)

#define TID_IDLE 0

// Preempt a program after 5 time slices
#define MAX_SLICES 5

typedef int32_t tgid_t;
typedef int32_t tid_t;

struct sched_entity {
	struct rb_node node;
	uint64_t vruntime; // Key for red-black tree
	uint32_t num_slices; // Number of time slices since was last preempted
};

struct task {
	// Careful not to move these as they are referenced in asm
	virtaddr_t rsp_top;
	virtaddr_t rsp_original;
	struct mmu_info *mmu; // For kernel threads, this is &kernel_mmu
	uint64_t flags;

	// Process state
	enum task_state {
		TASK_S_NOT_STARTED, // Not started yet
		TASK_S_RUNNABLE, // Either running or ready to be run (running tasks have TASK_RUNNING flag set)
		TASK_S_WAITING, // On a wait queue somewhere (e.g has acquired a mutex)
		TASK_S_SLEEPING, // To be woken up at a specific time
		TASK_S_ZOMBIE, // Waiting to be reaped by parent
		TASK_S_DEAD // Totally dead and ought to be removed
	} state;

	// Task identifiers
	tid_t tid;
	tgid_t tgid;

	// VFS information
	struct path *root, *cwd;

	cpuset_t affinity; // Defines which processors this task can run on

	struct sched_entity sched;
};

// Per-CPU task list
struct runq {
	spinlock_t lock;
	struct task *idle; // Idle thread for this CPU
	struct rbtree tree; // List of threads on the run queue
	uint64_t num_threads; // Number of threads running on this CPU
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
void sched_init(void);
void sched_run_bsp(void);
void sched_run_ap(void);
void sched_yield(void); // Called when a task voluntarily gives up control of the CPU
void sched_yield_preempt(void); // Called when a task is preempted
void sched_add(struct task *t);

struct task *task_fork(struct task *parent, virtaddr_t entry, uint64_t flags, const struct callee_regs *regs);
void task_execve(virtaddr_t function, char *argv[], unsigned int flags);
void task_wakeup(struct task *t);
void task_exit(struct task *t, int code);

// Arguments to kthreads are passed in rbx
#define create_kthread(entry, arg) ({ \
	struct callee_regs tmp = { \
		.rbx = (uint64_t)(arg) \
	}; \
	task_fork(current, (entry), FORK_KTHREAD, &tmp); })

void runq_init(struct task *);
void runq_add(struct task *);
void runq_remove(struct task *);
void runq_balance_pull(void);
struct task *runq_next(void);

void idle_task(void);

void init_kernel(void);
