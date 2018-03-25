#include "libk.h"
#include "syscall.h"
#include "proc.h"
#include "percpu.h"

#define NAME(name) syscall_ ## name
#define CAST(name) (syscall_t)NAME(name)

static int64_t NAME(yield)(void)
{
	schedule();
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
	schedule();
	return 1; // TODO: Return child PID
}

syscall_t syscall_table[NUM_SYSCALLS] = {
	CAST(yield),
	CAST(write),
	CAST(fork)
};
