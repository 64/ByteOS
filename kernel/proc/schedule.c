#include "syscall.h"
#include "libk.h"
#include "proc.h"
#include "percpu.h"
#include "mm.h"

static struct task dummy;

void schedule(void)
{
	// TODO: Disable preemption
	struct task *next = runq_next();
	//klog("sched", "Switching to %p\n", next);
	kassert_dbg(next->state == TASK_RUNNABLE || next->state == TASK_RUNNING);
	next->state = TASK_RUNNING;
	current->state = TASK_RUNNABLE;
	switch_to(next);
}

void sched_add(struct task *t)
{
	runq_add(t);
}

void sched_yield(void)
{
	schedule();
}

static void utask_entry(void)
{
	volatile int x = 0;
	execute_syscall(SYSCALL_FORK, FORK_UTHREAD, 0, 0, 0);
	x++;
	execute_syscall(SYSCALL_WRITE, '0' + x, 0, 0, 0);
	execute_syscall(SYSCALL_EXIT, 0, 0, 0, 0);
}

// The first kernel thread. Perform advanced initialisation (e.g forking) from here.
// Ends with a call to execve, beginning the first user process.
static void ktask_entry(void)
{
	runq_start_balancer();
	task_execve(utask_entry, NULL, 0);
}

static void init_dummy(struct task *t)
{
	memset(t, 0, sizeof *t);
	cpuset_set_id(&t->affinity, 0, 1);
	t->mmu = &kernel_mmu;
}

void sched_run(void)
{
	klog("sched", "Starting scheduler...\n");
	runq_init();
	percpu_set(current_task, &dummy);
	init_dummy(&dummy);
	struct task *t = task_fork(&dummy, ktask_entry, FORK_KTHREAD, NULL);
	task_wakeup(t);
	sched_yield();
}

