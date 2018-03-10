#include "proc.h"
#include "mm.h"
#include "libk.h"
#include "util.h"

extern void ret_from_ufork(void);
extern void ret_from_kfork(void);

static const struct callee_regs default_regs = {
	0, 0, 0, 0, 0, 0
};

#define TASK_KSTACK_ORDER 1
#define TASK_KSTACK_PAGES (1 << TASK_KSTACK_ORDER)
#define TASK_KSTACK_SIZE (TASK_KSTACK_PAGES * PAGE_SIZE)

#if 0
void task_init(struct task *task, virtaddr_t entry, uint64_t flags)
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
		for (size_t i = 0; i < 2; i++) {
			size_t off = i * PAGE_SIZE;
			uintptr_t page = (uintptr_t)page_to_virt(pmm_alloc_order(0, GFP_NONE));
			vmm_map_page(task->mmu, virt_to_phys((virtaddr_t)(page + off)),
					(virtaddr_t)(0x2000 + off), PAGE_WRITABLE | PAGE_USER_ACCESSIBLE);
		}
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
#endif

static struct page_table *clone_pgtab(struct page_table *pgtab, size_t level)
{
	if (level == 0) {
		// We are copying a physical page, not a page table
		virtaddr_t dest = page_to_virt(pmm_alloc_order(0, GFP_NONE));
		virtaddr_t src = pgtab;
		memcpy(dest, src, PAGE_SIZE);
		return dest;
	}

	struct page_table *rv = page_to_virt(pmm_alloc_order(0, GFP_NONE));
	size_t end_index = 512;
	if (level == 4) {
		memcpy(rv, kernel_mmu.p4, sizeof(struct page_table));
		end_index = (1 << 7);
	}

	for (size_t i = 0; i < end_index; i++) {
		if (pgtab->pages[i] & PAGE_PRESENT) {
			physaddr_t pgtab_phys = (physaddr_t)(pgtab->pages[i] & PTE_ADDR_MASK);
			virtaddr_t pgtab_virt = phys_to_virt(pgtab_phys);
			uint64_t flags = pgtab->pages[i] & ~PTE_ADDR_MASK;
			rv->pages[i] = virt_to_phys(clone_pgtab(pgtab_virt, level - 1)) | flags;
		}
	}

	return rv;
}

// TODO: Copy on write page table mappings
static struct mmu_info *clone_mmu(struct mmu_info *pmmu)
{
	// Allocate an mmu struct
	struct mmu_info *mmu = kmalloc(sizeof(struct mmu_info), KM_NONE);
	mmu->p4 = clone_pgtab(pmmu->p4, 4);
	return mmu;
}

struct task *task_fork(struct task *parent, virtaddr_t entry, uint64_t flags, const struct callee_regs *regs)
{
	struct task *t = kmalloc(sizeof(struct task), KM_NONE);
	t->flags = parent->flags;

	// Allocate a kernel stack
	uintptr_t kstack = TASK_KSTACK_SIZE + (uintptr_t)page_to_virt(pmm_alloc_order(TASK_KSTACK_ORDER, GFP_NONE));
	uint64_t *stack = (uint64_t *)kstack;
	t->rsp_original = (virtaddr_t)kstack;

	// Copy MMU information and set up the kernel stack
	if (flags & TASK_KTHREAD) {
		if (regs == NULL)
			regs = &default_regs;
		t->mmu = NULL;	
		t->flags |= TASK_KTHREAD;
		*--stack = (uint64_t)entry;
		*--stack = (uint64_t)ret_from_kfork; // Where switch_to will return
	} else {
		kassert_dbg(regs != NULL);
		t->flags &= ~(TASK_KTHREAD);
		t->mmu = clone_mmu(parent->mmu);

		// Set up simulated iret frame
		*--stack = 0x20 | 0x3; // ss
		*--stack = 0x4000; // rsp
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
	return t;
}
