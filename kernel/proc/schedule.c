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
	kassert_dbg(next != current);
	kassert_dbg(next->state == TASK_RUNNABLE);
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
	volatile uint64_t var = 0;
	if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) > 0) { // Fork
		execute_syscall(SYSCALL_WRITE, 'C', 0, 0, 0); // Write
		execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0); // Yield
		var = 3;
	} else {
		if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) > 0) { // Fork
			execute_syscall(SYSCALL_WRITE, 'B', 0, 0, 0); // Write
			execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0); // Yield
			var = 1;
		} else {
			execute_syscall(SYSCALL_WRITE, 'A', 0, 0, 0); // Write
			execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0); // Yield
			var = 2;
		}
	}
	execute_syscall(SYSCALL_WRITE, '0' + (uint8_t)var, 0, 0, 0); // Write
	execute_syscall(SYSCALL_EXIT, 0, 0, 0, 0); // Exit
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
