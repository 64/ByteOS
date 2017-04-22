#include <memory/kheap.h>
#include <system/task.h>
#include <klog.h>

static struct task *current_task;
static struct task main_task;
static struct task other_task;

static void other_main(void) {
	klog_info("Hello multitasking world!\n");
	task_preempt();
}

void tasking_init(void) {
	asm volatile (
		"movl %%cr3, %%eax; movl %%eax, %0;"
		: "=m"(main_task.regs.cr3)
		:
		: "%eax"
	);
	asm volatile (
		"pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;"
		: "=m"(main_task.regs.eflags)
		:
		: "%eax"
	);

	task_create(&other_task, other_main, main_task.regs.eflags, (uint32_t*)main_task.regs.cr3);
	main_task.next = &other_task;
	other_task.next = &main_task;
	current_task = &main_task;
}

void task_create(struct task *task, void(*main)(), uint32_t flags, uint32_t *pagedir) {
	task->regs.eax = 0;
	task->regs.ebx = 0;
	task->regs.ecx = 0;
	task->regs.edx = 0;
	task->regs.esi = 0;
	task->regs.edi = 0;
	task->regs.eflags = flags;
	task->regs.eip = (uint32_t)main;
	task->regs.cr3 = (uint32_t)pagedir;
	task->regs.esp = (uint32_t)((uint8_t*)kmalloc_a(0x1000) + 0x1000);
	task->next = NULL;
}

void task_test() {
	klog_info("Switching to other task...\n");
	task_preempt();
	klog_info("Returned to main task.\n");
}

void task_preempt() {
	struct task *last = current_task;
	current_task = current_task->next;
	task_switch(&last->regs, &current_task->regs);
}
