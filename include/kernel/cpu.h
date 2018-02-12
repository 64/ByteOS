#pragma once

#include <stdint.h>

struct interrupt_frame {
	uint64_t int_no, err_code;
	uint64_t rip, cs, rflags, rsp, ss, __salign;
};

struct percpu {
	struct task *task; // Currently running task
	uint64_t rsp_scratch;
	uint32_t id;
};

void cpu_local_init(void);
void cpu_local_set_task(struct task *);

void irq_handler(struct interrupt_frame *);
void exception_handler(struct interrupt_frame *);

static inline uint64_t msr_read(uint64_t msr)
{
	uint64_t rv_low, rv_high;
	asm volatile (
		"rdmsr"
		: "=d"(rv_high), "=a"(rv_low)
		: "c"(msr)
	);
	return rv_low | ((rv_high << 32) & 0xFFFFFFFF00000000);
}

static inline void msr_write(uint64_t msr, uint64_t value)
{
	asm volatile (
		"wrmsr"
		:
		: "c"(msr), "a"(value & 0xFFFFFFFF), "d"((value >> 32) & 0xFFFFFFFF)
	);
}
