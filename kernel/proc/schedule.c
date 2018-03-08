#include "syscall.h"
#include "libk.h"
#include "proc.h"
#include "mm.h"

extern void ret_from_fork(void);

static struct task task1, task2, dummy;
static struct task *current, *next;

void schedule(void)
{
	struct task *tmp = current;
	current = next;
	next = tmp;
	switch_to(current);
}

static void task_init(struct task *task, virtaddr_t entry, uint64_t flags)
{
	if (flags & TASK_KTHREAD) {
		task->mmu = NULL;	
	} else {
		task->mmu = kmalloc(sizeof(struct mmu_info), KM_NONE);	
		task->mmu->p4 = page_to_virt(pmm_alloc_order(0, GFP_NONE));
		memcpy(task->mmu->p4, kernel_mmu.p4, PAGE_SIZE);
		vmm_map_page(task->mmu, kern_to_phys(entry), (virtaddr_t)0x1000, PAGE_EXECUTABLE | PAGE_USER_ACCESSIBLE);
	}

	uint64_t *stack = page_to_virt(pmm_alloc_order(1, GFP_NONE));
	stack = (uint64_t *)((uintptr_t)stack + (1 << 1) * PAGE_SIZE);
	task->rsp_original = stack;
	if (flags & TASK_KTHREAD)
		*--stack = (uint64_t)entry;
	else {
		uintptr_t user_stack = (uintptr_t)page_to_virt(pmm_alloc_order(1, GFP_NONE));
		vmm_map_page(task->mmu, virt_to_phys((virtaddr_t)user_stack), (virtaddr_t)0x2000, PAGE_WRITABLE | PAGE_USER_ACCESSIBLE);
		vmm_map_page(task->mmu, virt_to_phys((virtaddr_t)(user_stack + PAGE_SIZE)), (virtaddr_t)0x3000, PAGE_WRITABLE | PAGE_USER_ACCESSIBLE);
		user_stack += (1 << 1) * PAGE_SIZE;
		// Set up iret frame
		*--stack = 0x20 | 0x3; // ss
		*--stack = 0x4000; // rsp
		*--stack = read_rflags() | 0x200; // rflags with interrupts enabled
		*--stack = 0x28 | 0x3; // cs
		*--stack = 0x1000 + ((uintptr_t)entry & (PAGE_SIZE - 1)); // rip
	}
	// switch_to stuff
	*--stack = (uint64_t)ret_from_fork; // Return rip
	*--stack = 0; // rbx
	*--stack = 0; // rbp
	*--stack = 0; // r12
	*--stack = 0; // r13
	*--stack = 0; // r14
	*--stack = 0; // r15
	task->rsp_top = (virtaddr_t)stack;
	task->flags = flags;
}

static void task_1(void)
{
	while (1) {
		for (size_t i = 0; i < 100; i++) {
			execute_syscall(1, '1', 0, 0, 0);
		}
		execute_syscall(0, 0, 0, 0, 0); // Yield
	}
}

static void task_2(void)
{
	while (1) {
		for (size_t i = 0; i < 100; i++) {
			execute_syscall(1, '2', 0, 0, 0);
		}
		execute_syscall(0, 0, 0, 0, 0); // Yield
	}
}

void sched_run(void)
{
	asm volatile (
		"movq %0, %%gs:0"
		:
		: "i"(&dummy)
	);
	task_init(&task1, task_1, TASK_NONE);
	task_init(&task2, task_2, TASK_NONE);
	current = &task1;
	next = &task2;
	klog("sched", "Starting scheduler...\n");
	schedule();
}

