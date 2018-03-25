#include <stdint.h>

#define NUM_SYSCALLS 4
#define SYSCALL_ERROR 0xFFFFFFFFFFFFFFFFULL

typedef int64_t (*syscall_t)(void);

// Definition for syscall jump table
extern syscall_t syscall_table[NUM_SYSCALLS];

static inline int64_t execute_syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
	// Use dummy output variables because we need to clobber the input ones
	// https://stackoverflow.com/questions/48381184/can-i-modify-input-operands-in-gcc-inline-assembly
	int64_t rv;
	uint64_t dummy_rdi, dummy_rsi, dummy_rdx, dummy_rcx;
	asm volatile (
		"syscall\n"
		: "=a"(rv), "=D"(dummy_rdi), "=S"(dummy_rsi), "=d"(dummy_rdx), "=c"(dummy_rcx)
		: "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "c"(arg4)
		: "r8", "r9", "r10", "r11"
	);
	return rv;
}
