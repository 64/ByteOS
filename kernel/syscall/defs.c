#include "libk.h"
#include "syscall.h"

#define NAME(name) syscall_ ## name
#define CAST(name) (syscall_t)NAME(name)

static uint64_t NAME(write)(uint64_t arg)
{
	kprintf("%s", (char *)arg);
	return 0;
}

static uint64_t NAME(yield)(void)
{
	//task_switch_fn();
	return 0;
}

syscall_t syscall_table[NUM_SYSCALLS] = {
	CAST(write),
	CAST(yield)
};



