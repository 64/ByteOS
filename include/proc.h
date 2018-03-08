#pragma once

#include "mm.h"

#define TASK_KTHREAD (1 << 0)
#define TASK_NONE 0

struct task {
	virtaddr_t rsp_top;
	struct mmu_info *mmu;
	uint64_t flags;
	virtaddr_t rsp_original;
};

void schedule(void);
void sched_run(void);
void switch_to(struct task *);
