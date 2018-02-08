#include <stdint.h>

#define NUM_SYSCALLS 1

struct syscall_frame {
	uint64_t rax, rdi, rsi; // Add more parameters as appropriate
};

typedef uint64_t (*syscall_t)(struct syscall_frame *);

// Definition for syscall jump table
extern syscall_t syscall_table[NUM_SYSCALLS];
