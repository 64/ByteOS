#pragma once

#include <stdint.h>

#include "types.h"

struct context {
	uint64_t rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t rflags, rip;
	uint64_t cs, ss;
	uint64_t cr3;
};

struct task {
	struct context ctx;
	virtaddr_t kernel_stack;
};

__attribute__((noreturn))
void switch_to(struct task *);

__attribute__((noreturn))
void schedule(struct context *);

void task_switch_fn(void);

void run_tasks(void);
