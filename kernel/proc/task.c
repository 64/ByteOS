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
	while (1)
		kprintf("A");
}

static void task_two(void)
{
	while (1)
		kprintf("B");
}

void run_tasks(void)
{
	task_init(&task1, &task_one);
	task_init(&task2, &task_two);
	running = &task1;
	idle = &task2;
	context_switch(&running->ctx);
}
