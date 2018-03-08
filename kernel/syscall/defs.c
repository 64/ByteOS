#include "libk.h"
#include "syscall.h"
#include "proc.h"
#include "asm.h"

#define NAME(name) syscall_ ## name
#define CAST(name) (syscall_t)NAME(name)

static uint64_t NAME(yield)(void)
{
	schedule();
	return 0;
}

static uint64_t NAME(write)(uint64_t arg1)
{
	kprintf("%c", (int)arg1);
	return 0;
}

syscall_t syscall_table[NUM_SYSCALLS] = {
	CAST(yield),
	CAST(write)
};
