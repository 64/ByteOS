#include "libk.h"
#include "syscall.h"
#include "proc.h"
#include "asm.h"

#define NAME(name) syscall_ ## name
#define CAST(name) (syscall_t)NAME(name)

static int64_t NAME(yield)(void)
{
	schedule();
	return 0;
}

static int64_t NAME(write)(char c)
{
	kprintf("%c", c);
	return 0;
}

static int64_t NAME(fork)(uint64_t flags, struct callee_regs *regs, virtaddr_t return_addr)
{
	if (flags & TASK_KTHREAD)
		return -1;
	struct task *new = task_fork(current(), return_addr, flags, regs);
	panic("fork");
	switch_to(new);
	return 0;
}

syscall_t syscall_table[NUM_SYSCALLS] = {
	CAST(yield),
	CAST(write),
	CAST(fork)
};
