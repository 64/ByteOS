#include "libk.h"
#include "syscall.h"
#include "proc.h"
#include "percpu.h"

#define NAME(name) syscall_ ## name
#define CAST(name) (syscall_t)NAME(name)

static int64_t NAME(yield)(void)
{
	sched_yield();
	return 0;
}

static int64_t NAME(write)(char c)
{
	kprintf("%c\n", c);
	return 0;
}

static int64_t NAME(fork)(uint64_t flags, struct callee_regs *regs, virtaddr_t return_addr)
{
	if (flags & TASK_KTHREAD)
		return -1;
	struct task *child = task_fork(percpu_get(current), return_addr, flags, regs);
	task_wakeup(child);
	sched_yield();
	return child->pid;
}

static int64_t NAME(exit)(int code)
{
	task_exit(percpu_get(current), code);
	panic("exit returned");
	return -1;
}

syscall_t syscall_table[NUM_SYSCALLS] = {
	CAST(yield),
	CAST(write),
	CAST(fork),
	CAST(exit)
};
