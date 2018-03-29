#include "libk.h"
#include "syscall.h"
#include "proc.h"
#include "percpu.h"

#include "gen/syscall_gen.c"

#define NAME(name) syscall_ ## name

int64_t syscall_sched_yield(void)
{
	sched_yield();
	return 0;
}

int64_t syscall_write(char c)
{
	kprintf("%c\n", c);
	return 0;
}

int64_t syscall_fork(uint64_t flags, struct callee_regs *regs, virtaddr_t return_addr)
{
	if (flags & TASK_KTHREAD)
		return -1;
	struct task *child = task_fork(percpu_get(current), return_addr, flags, regs);
	task_wakeup(child);
	sched_yield();
	return child->pid;
}

int64_t syscall_exit(int code)
{
	task_exit(percpu_get(current), code);
	panic("exit returned");
	return -1;
}
