#pragma once

#include <stdint.h>

typedef uintptr_t physaddr_t;
typedef void *virtaddr_t;
typedef void *kernaddr_t;

struct interrupt_frame {
	uint64_t int_no, err_code;
	uint64_t rip, cs, rflags, rsp, ss, __salign;
};

void irq_ack(int int_no);
