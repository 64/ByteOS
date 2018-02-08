#include "proc.h"
#include "libk.h"
#include "mm.h"

struct task task1, task2;
struct task *running, *idle;

static void task_init(struct task *t, virtaddr_t entry)
{
	memset(&t->ctx, 0, sizeof(t->ctx));
	t->ctx.rsp = (uint64_t)page_to_virt(pmm_alloc_order(0, 0));
	t->ctx.cr3 = (uint64_t)kern_to_phys(kernel_p4);
	t->ctx.rip = (uint64_t)entry;
	t->ctx.cs = 0x8;  // Kernel code selector
	t->ctx.ss = 0x10; // Kernel data selector
}

static void task_one(void)
{
	while (1) {
		size_t i = 0;
		while (i++ < 100)
			kprintf("A");
		task_switch_fn();
	}
}

static void task_two(void)
{
	while (1) {
		size_t i = 0;
		while (i++ < 100)
			kprintf("B");
		task_switch_fn();
	}
}

void schedule(struct context *ctx)
{
	memcpy(&running->ctx, ctx, sizeof *ctx);
	struct task *tmp = idle;
	idle = running;
	running = tmp;
	switch_to(&running->ctx);
}

void run_tasks(void)
{
	task_init(&task1, &task_one);
	task_init(&task2, &task_two);
	running = &task1;
	idle = &task2;
	switch_to(&running->ctx);
}
