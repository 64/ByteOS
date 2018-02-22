#pragma once

#include <stdint.h>

#include "types.h"

struct percpu {
	struct task *task; // Currently running task
	uint64_t rsp_scratch;
	uint32_t cpu_id;
};

void percpu_init(void);
