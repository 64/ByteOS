#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

struct percpu {
	struct task *task; // Currently running task
	virtaddr_t rsp_scratch;
	uint32_t id;
	bool need_reschedule;
};

void percpu_init(void);
