#pragma once

#include <stdint.h>

struct iret_frame {
	// Pushed by the processor on interrupt, iretq reads these
	uint64_t rip, cs, rflags, rsp, ss;
};

__attribute__((noreturn))
void context_switch(struct iret_frame *);
