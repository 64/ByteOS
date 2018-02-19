#pragma once

#include <stdint.h>

#include "types.h"

struct stack_regs {
	// These registers are not generally saved by the caller
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t rbp;
	uint64_t rbx;
	// These registers are always saved when entering C code
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	// Contains error code and interrupt number for exceptions
	// Contains syscall number for syscalls
	// Contains just the interrupt number otherwise
	uint64_t info;
	// Interrupt stack frame
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

struct percpu {
	struct task *task; // Currently running task
	uint64_t rsp_scratch;
	uint32_t cpu_id;
};

void percpu_init(void);
