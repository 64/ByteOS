#include "syscall.h"
#include "libk.h"
#include "proc.h"
#include "percpu.h"
#include "mm.h"

static struct task dummy;

void schedule(void)
{
	// TODO: Disable preemption
	// TODO: Switch to child if forked
	struct task *t = percpu_get(current);
	struct task *next = dlist_get_next(t, list);
	if (next == NULL)
		next = percpu_get(run_queue);
	//klog("sched", "Next = %p\n", next);
	kassert_dbg(next->state == TASK_RUNNABLE);
	//klog("sched", "Switching to %p\n", next);
	next->state = TASK_RUNNING;
	t->state = TASK_RUNNABLE;
	switch_to(next);
}

void sched_add(struct task *t)
{
	// TODO: Disable preemption
	struct task *run_queue = percpu_get(run_queue);
	if (run_queue == NULL) {
		percpu_set(run_queue, t);
		dlist_set_next(t, list, (struct task *)NULL);
		dlist_set_next(percpu_get(current), list, t);
		t->list.prev = NULL;
	} else
		dlist_append(run_queue, list, t);
	//klog("sched", "Added task at %p\n", t);
}

static void utask_entry(void)
{
	if (execute_syscall(2, 0, 0, 0, 0) > 0) // Fork
		execute_syscall(1, 'P', 0, 0, 0); // Write
	else
		execute_syscall(1, 'C', 0, 0, 0); // Write
	while (1)
		execute_syscall(0, 0, 0, 0, 0); // Yield
}

static void ktask_entry(void)
{
	task_execve(utask_entry, NULL, 0);
}

void sched_run(void)
{
	klog("sched", "Starting scheduler...\n");
	percpu_set(run_queue, NULL);
	percpu_set(current, &dummy);
	task_fork(&dummy, ktask_entry, TASK_KTHREAD, NULL);
	schedule();
}

