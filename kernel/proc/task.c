#include "proc.h"
#include "libk.h"
#include "mm.h"
#include "syscall.h"
#include "util.h"

struct task task1, task2;
struct task *running, *idle;

static virtaddr_t task_get_p4(bool kernel)
{
	if (kernel)
		return kernel_p4;
	struct page *p = pmm_alloc_order(0, 0);
	virtaddr_t p4_virt = page_to_virt(p);
	memcpy(p4_virt, kernel_p4, PAGE_SIZE); // Copy the mappings
	return p4_virt;
}

static UNUSED() void dump_task(struct task *t)
{
	kprintf("rip: %p\nrsp: %p\nkernel rsp: %p\n", (void *)t->ctx.rip, (void *)t->ctx.rsp, t->kernel_stack);
}


static void task_init(struct task *t, virtaddr_t entry, bool kernel)
{
	memset(&t->ctx, 0, sizeof(t->ctx));
	virtaddr_t user_stack = page_to_virt(pmm_alloc_order(0, 0));
	struct page_table *task_p4 = task_get_p4(kernel);
	t->ctx.cr3 = virt_to_phys(task_p4);
	t->ctx.rip = 0x100000 + ((uintptr_t)entry & 0xFFF);
	if (kernel) {
		t->ctx.cs = 0x8;  // Kernel code selector
		t->ctx.ss = 0x10; // Kernel data selector
	} else {
		t->ctx.cs = 0x28 | 0x03; // 0x28 User 64-bit code selector, RPL 3
		t->ctx.ss = 0x20 | 0x03; // 0x20 User data selector, RPL 3
	}
	paging_map_page(task_p4, ALIGNDOWN(kern_to_phys(entry), PAGE_SIZE), (virtaddr_t)0x100000, PAGE_EXECUTABLE);
	paging_map_page(task_p4, virt_to_phys(user_stack), (virtaddr_t)0x200000, PAGE_WRITABLE);
	t->ctx.rsp = 0x200000 + PAGE_SIZE;
	t->kernel_stack = (virtaddr_t)((uintptr_t)page_to_virt(pmm_alloc_order(0, 0)) + PAGE_SIZE);
	dump_task(t);
}

static void task_one(void)
{
	while (1) {
		size_t i = 0;
		while (i++ < 100)
			execute_syscall(0, (uint64_t)"0", 0, 0, 0); // Write
		execute_syscall(1, 0, 0, 0, 0);	// Yield
	}
}

static void task_two(void)
{
	while (1) {
		size_t i = 0;
		while (i++ < 100)
			execute_syscall(0, (uint64_t)"1", 0, 0, 0); // Write
		execute_syscall(1, 0, 0, 0, 0);	// Yield
	}
}

void schedule(struct context *ctx)
{
	memcpy(&running->ctx, ctx, sizeof(struct context));
	struct task *tmp = idle;
	idle = running;
	running = tmp;
	switch_to(running);
}

void run_tasks(void)
{
	task_init(&task1, &task_one, false);
	task_init(&task2, &task_two, false);
	running = &task1;
	idle = &task2;
	switch_to(running);
}
