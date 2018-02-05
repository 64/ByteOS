#pragma once

#include <stdint.h>

typedef uintptr_t physaddr_t;
typedef void *virtaddr_t;
typedef void *kernaddr_t;

struct interrupt_frame {
	uint64_t r11, r10, r9, r8;
	uint64_t rdi, rsi, rdx, rcx, rax;
	uint64_t int_no, err_code;
	uint64_t rip, cs, rflags, rsp, ss, salign;
};

void irq_ack(int int_no);
