#include "syscall.h"

#define DECL(name) static uint64_t name(struct syscall_frame *)

DECL(test_syscall_zero);

syscall_t syscall_table[NUM_SYSCALLS] = {
	test_syscall_zero
};

static uint64_t test_syscall_zero(struct syscall_frame *frame)
{
	(void)frame;
	return 0;
}


