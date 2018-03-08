#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

struct percpu {
	struct task *task; // Currently running task
	virtaddr_t rsp_scratch;
	bool need_reschedule;
	virtaddr_t tss;
	uint32_t id;
};

void percpu_init(void);
