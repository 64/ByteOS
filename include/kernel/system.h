#pragma once

#include <stdint.h>

typedef uintptr_t physaddr_t;
typedef void *virtaddr_t;
typedef void *kernaddr_t;

struct interrupt_frame {
	uint64_t int_no, err_code;
	uint64_t rip, cs, rflags, rsp, ss, __salign;
};

struct percpu {
	struct task *task; // Currently running task
	uint64_t rsp_scratch;
	uint32_t id;
};

void irq_ack(int int_no);
void cpu_local_init(void);
void cpu_local_set_task(struct task *);
