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

	// Remove 'need preempt' flag
	percpu_set(reschedule, false);

	// Update scheduler statistics for the previous task
	current->sched.vruntime++; // Note that current may be the idle task here

	// Reinsert current into the rbtree
	if (current->tid != TID_IDLE)
		runq_add(current);

	// Get next task
	struct task *next = runq_next();
	if (next->tid == TID_IDLE) {
		// We're about to go idle, run the balancer and try again
		runq_balance_pull();
		next = runq_next();
	}

	kassert_dbg(next->state == TASK_S_RUNNABLE);

	// Swap the states of the tasks
	current->flags &= ~TASK_RUNNING;
	next->flags |= TASK_RUNNING;
	next->flags &= ~TASK_PREEMPTED;

	// Pre-emption is re-enabled in switch_to
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

void sched_yield_preempt(void)
{
	preempt_inc();
	current->flags |= TASK_PREEMPTED;
	preempt_dec();

	schedule();
}

void sched_init(void)
{
	// Initialise the dummy task since every task switch requires a 'current' task to switch from
	memset(&dummy, 0, sizeof dummy);
	dummy.mmu = &kernel_mmu;
	dummy.tid = TID_IDLE;
	percpu_set(current_task, &dummy);
}

static inline void wait_for_schedulers(void)
{
	atomic_inc_read32(&schedulers_waiting);
	while (atomic_read32(&schedulers_waiting) < smp_nr_cpus())
		;

	klog("sched", "Starting scheduler for CPU %d...\n", smp_cpu_id());
}

void sched_run_bsp(void)
{
	// Create the init task
	runq_init(&dummy);

	struct task *t = task_fork(&dummy, init_kernel, FORK_KTHREAD, NULL);
	task_wakeup(t);

	wait_for_schedulers();

	lapic_timer_enable();
	// TODO: Possible race condition here, if we get rescheduled before sched_yield happens
	sched_yield();
}

void sched_run_ap(void)
{
	percpu_set(current_task, &dummy);

	runq_init(&dummy);	

	wait_for_schedulers();

	lapic_timer_enable();
	// TODO: Possible race condition here, if we get rescheduled before sched_yield happens
	sched_yield();
}
