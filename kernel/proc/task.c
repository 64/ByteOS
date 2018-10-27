#include "proc.h"
#include "mm.h"
#include "libk.h"
#include "percpu.h"
#include "util.h"

extern void ret_from_ufork(void);
extern void ret_from_kfork(void);
extern void __attribute__((noreturn)) ret_from_execve(virtaddr_t entry, uint64_t rsp);

#define TASK_KSTACK_ORDER 1
#define TASK_KSTACK_PAGES (1 << TASK_KSTACK_ORDER)
#define TASK_KSTACK_SIZE (TASK_KSTACK_PAGES * PAGE_SIZE)

static atomic32_t next_tid = { 0 };

static const struct callee_regs default_regs = { 0 };

struct task *task_fork(struct task *parent, virtaddr_t entry, uint64_t clone_flags, const struct callee_regs *regs)
{
	if (parent == NULL)
		parent = current;

	kassert_dbg(!((clone_flags & FORK_KTHREAD) && (clone_flags & FORK_UTHREAD)));

	struct task *t = kmalloc(sizeof(struct task), KM_NONE);
	memset(t, 0, sizeof *t);
	t->flags = parent->flags;
	t->tid = atomic_inc_read32(&next_tid);
	t->tgid = t->tid; // TODO

	klog_verbose("task", "Forked PID %d to create PID %d\n", parent->tid, t->tid);

	// Allocate a kernel stack
	uintptr_t kstack = TASK_KSTACK_SIZE + (uintptr_t)page_to_virt(pmm_alloc_order(TASK_KSTACK_ORDER, GFP_NONE));
	uint64_t *stack = (uint64_t *)kstack;
	// TODO: Remove this variable. We can work out the stack top by masking rsp
	// given that the kernel stack size is fixed at compile time, and allocs are aligned.
	t->rsp_original = (virtaddr_t)kstack;

	// Copy MMU information and set up the kernel stack
	if (clone_flags & FORK_KTHREAD) {
		if (regs == NULL)
			regs = &default_regs;
		t->mmu = &kernel_mmu;	
		t->flags |= TASK_KTHREAD;
		*--stack = (uint64_t)entry;
		*--stack = regs->rbx; // Argument passed to the thread
		*--stack = (uint64_t)ret_from_kfork; // Where switch_to will return
	} else {
		kassert_dbg(regs != NULL);
		t->flags &= ~(TASK_KTHREAD);

		if (clone_flags & FORK_UTHREAD) {
			mmu_inc_users(parent->mmu);
			t->mmu = parent->mmu;
		} else {
			t->mmu = mmu_alloc();
			mmu_clone_cow(t->mmu, parent->mmu);
		}

		// Set up simulated iret frame
		*--stack = 0x20 | 0x3; // ss
		*--stack = regs->rsp; // rsp
		*--stack = read_rflags() | 0x200; // rflags with interrupts enabled
		*--stack = 0x28 | 0x3; // cs
		*--stack = (uint64_t)entry; // rip
		*--stack = (uint64_t)ret_from_ufork; // Where switch_to will return
	}

	*--stack = regs->rbx;
	*--stack = regs->rbp;
	*--stack = regs->r12;
	*--stack = regs->r13;
	*--stack = regs->r14;
	*--stack = regs->r15;
	t->rsp_top = (virtaddr_t)stack;

	cpuset_copy(&t->affinity, &parent->affinity);

	// Add the task to the scheduler
	t->state = TASK_S_NOT_STARTED;
	return t;
}

void task_wakeup(struct task *t)
{
	if (t->state != TASK_S_RUNNABLE) {
		t->state = TASK_S_RUNNABLE;
		sched_add(t);
	}
}

void task_exit(struct task *t, int UNUSED(code))
{
	if (t->state == TASK_S_RUNNABLE)
		runq_remove(t);

	t->state = TASK_S_ZOMBIE;

	mmu_dec_users(t->mmu);

	if (t == current) {
		// TODO: Put ourselves on scheduler cleanup list if necessary
		sched_yield();
		panic("Schedule returned at the end of task_exit");
	}
}

void __attribute__((noreturn)) task_execve(virtaddr_t function, char UNUSED(*argv[]), unsigned int UNUSED(flags))
{
	struct task *self = current;
	if (self->mmu == &kernel_mmu)
		self->mmu = mmu_alloc();
	else
		mmu_dec_users(self->mmu);

	mmu_init(self->mmu);

#define UTASK_ENTRY 0x1000
#define UTASK_STACK_BOTTOM 0x3000
#define UTASK_STACK_TOP 0x5000

	// Set up the entry point
	uintptr_t entry = UTASK_ENTRY;
	vmm_map_page(self->mmu, kern_to_phys(function), (virtaddr_t)entry, PAGE_EXECUTABLE | PAGE_USER_ACCESSIBLE);
	vmm_map_page(self->mmu, kern_to_phys(function) + PAGE_SIZE, (virtaddr_t)(entry + PAGE_SIZE), PAGE_EXECUTABLE | PAGE_USER_ACCESSIBLE);
	entry += ((uintptr_t)function & 0xFFF);

	for (size_t i = 0; i < 2; i++) {
		size_t off = i * PAGE_SIZE;
		virtaddr_t page = page_to_virt(pmm_alloc_order(0, GFP_NONE));
		vmm_map_page(self->mmu, virt_to_phys(page), (virtaddr_t)(UTASK_STACK_BOTTOM + off), PAGE_WRITABLE | PAGE_USER_ACCESSIBLE);
	}

	// Create a vm_area for the code
	struct vm_area *code = kmalloc(sizeof(struct vm_area), KM_NONE);
	memset(code, 0, sizeof *code);
	code->base = UTASK_ENTRY;
	code->len = 2 * PAGE_SIZE;
	code->type = VM_AREA_TEXT;
	code->flags = VM_AREA_EXECUTABLE;
	area_add(self->mmu, code);

	// Create a vm_area for the stack
	struct vm_area *stack = kmalloc(sizeof(struct vm_area), KM_NONE);
	memset(stack, 0, sizeof *stack);
	stack->base = UTASK_STACK_TOP;
	stack->len = UTASK_STACK_TOP - UTASK_STACK_BOTTOM;
	stack->type = VM_AREA_STACK;
	stack->flags = VM_AREA_WRITABLE;
	area_add(self->mmu, stack);

	slist_foreach(cur, list, self->mmu->areas) {
		klog_verbose("task", "Added area at %p, size %zu\n", (void *)cur->base, cur->len);
	}

	// Switch to the new cr3.
	// Disable preemption so the cpuset is always correct.
	preempt_inc();
	mmu_update_cpuset(self->mmu, percpu_get(id), 1);
	write_cr3(virt_to_phys(self->mmu->p4));
	preempt_dec();

	ret_from_execve((virtaddr_t)entry, UTASK_STACK_TOP);
}
