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
	//klog("sched", "Next = %p\n", next);
	kassert_dbg(next->state == TASK_RUNNABLE);
	//klog("sched", "Switching to %p\n", next);
	next->state = TASK_RUNNING;
	percpu_get(current)->state = TASK_RUNNABLE;
	switch_to(next);
}

void sched_add(struct task *t)
{
	runq_add(t);
}

static void utask_entry(void)
{
	uint64_t var = 0;
	// Fork
	if (execute_syscall(2, 0, 0, 0, 0) > 0) {
		execute_syscall(1, 'A', 0, 0, 0); // Write
		execute_syscall(0, 0, 0, 0, 0); // Yield
		var = 3;
	} else {
		if (execute_syscall(2, 0, 0, 0, 0) > 0) {
			execute_syscall(1, 'B', 0, 0, 0); // Write
			execute_syscall(0, 0, 0, 0, 0); // Yield
			var = 1;
		} else {
			execute_syscall(1, 'C', 0, 0, 0); // Write
			execute_syscall(0, 0, 0, 0, 0); // Yield
			var = 2;
		}
	}
	execute_syscall(1, '0' + (uint8_t)var, 0, 0, 0);
	while (1)
		execute_syscall(0, 0, 0, 0, 0); // Yield
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
	percpu_set(current, &dummy);
	init_dummy(&dummy);
	struct task *t = task_fork(&dummy, ktask_entry, TASK_KTHREAD, NULL);
	task_wakeup(t);
	schedule();
}

