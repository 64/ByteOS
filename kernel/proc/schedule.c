#include "syscall.h"
#include "libk.h"
#include "proc.h"
#include "mm.h"

static struct task dummy;
static struct task *current_task, *next_task;

void schedule(void)
{
	if (next_task == NULL)
		return;
	struct task *tmp = current_task;
	current_task = next_task;
	next_task = tmp;
	switch_to(current_task);
}


static void __attribute__((unused)) utask_entry(void)
{
	if (execute_syscall(2, 0, 0, 0, 0) > 0)
		execute_syscall(1, 'P', 0, 0, 0);
	else
		execute_syscall(1, 'C', 0, 0, 0);
	while (1)
		;
}

static void ktask_entry(void)
{
	task_execve(utask_entry, NULL, 0);
}

void sched_run(void)
{
	asm volatile (
		"movq %0, %%gs:0"
		:
		: "i"(&dummy)
	);
	current_task = task_fork(&dummy, ktask_entry, TASK_KTHREAD, NULL);
	next_task = NULL;
	klog("sched", "Starting scheduler...\n");
	switch_to(current_task);
}

