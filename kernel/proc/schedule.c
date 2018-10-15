#include "syscall.h"
#include "libk.h"
#include "proc.h"
#include "percpu.h"
#include "mm.h"

static struct task dummy;
static atomic32_t schedulers_waiting;

void schedule(void)
{
	// Disable preemption
	preempt_inc();

	// Remove 'need preempt' flag (apart from idle task, which always needs preemption
	if ((current->flags & TASK_NEED_PREEMPT) && current->tid != 0)
		current->flags &= ~TASK_NEED_PREEMPT;

	// Update scheduler statistics for the previous task
	current->sched.vruntime++; // Note that current may be the idle task here

	// Reinsert current into the rbtree
	if (current->tid != 0)
		runq_add(current);

	// Get next task
	struct task *next = runq_next();
	if (next->tid == 0) {
		// We're about to go idle, run the balancer and try again
		runq_balance_pull();
		next = runq_next();
	}

	kassert_dbg(next->state == TASK_RUNNABLE || next->state == TASK_RUNNING);

	// Swap the states of the tasks
	current->state = TASK_RUNNABLE;
	next->state = TASK_RUNNING;

	// Re-enable preemption
	preempt_dec();

	// Do the context switch
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
	int x = 0;
	if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'A' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'a' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, '0' + (x++ % 10), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'A' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'a' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, '0' + (x++ % 10), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'A' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else if (execute_syscall(SYSCALL_FORK, 0, 0, 0, 0) == 0) {
		while (1) {
			execute_syscall(SYSCALL_WRITE, 'a' + (x++ % 26), 0, 0, 0);
			//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
		}
	} else {
		while (1)
			;//execute_syscall(SYSCALL_SCHED_YIELD, 0, 0, 0, 0);
	}
}

// The first kernel thread. Perform advanced initialisation (e.g forking) from here.
// Ends with a call to execve, beginning the first user process.
static void ktask_entry(void)
{
	task_execve(utask_entry, NULL, 0);
}

// Since every task switch requires a 'current' task to switch from, create an initial dummy task.
static void init_dummy(struct task *t)
{
	memset(t, 0, sizeof *t);
	t->mmu = &kernel_mmu;
	t->tid = 0; // So this task doesn't get inserted into the rbtree
}

void sched_run_bsp(void)
{
	klog("sched", "Starting scheduler for CPU 0...\n");
	percpu_set(current_task, &dummy);
	init_dummy(&dummy);

	struct task *t = task_fork(&dummy, ktask_entry, FORK_KTHREAD, NULL);
	cpuset_set_id(&t->affinity, 0, 1);

	runq_init(&dummy);
	task_wakeup(t);

	// Initialise the scheduler on APs
	lapic_send_ipi(0, IPI_BROADCAST | IPI_FIXED | IRQ_IPI_SCHED_HINT);

	// Wait for all schedulers to finish initialising
	atomic_inc_read32(&schedulers_waiting);
	while (atomic_read32(&schedulers_waiting) < smp_nr_cpus())
		;

	lapic_timer_enable();
	// TODO: Possible race condition here, if we get rescheduled before sched_yield happens
	sched_yield();
}

void sched_run_ap(void)
{
	klog("sched", "Starting scheduler for CPU %d...\n", smp_cpu_id());
	runq_init(&dummy);	
	percpu_set(current_task, &dummy);

	// Wait for all schedulers to finish initialising
	atomic_inc_read32(&schedulers_waiting);
	while (atomic_read32(&schedulers_waiting) < smp_nr_cpus())
		;

	lapic_timer_enable();
	// TODO: Possible race condition here, if we get rescheduled before sched_yield happens
	sched_yield();
}
