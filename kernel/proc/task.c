#include "proc.h"
#include "mm.h"
#include "libk.h"
#include "util.h"

extern void ret_from_ufork(void);
extern void ret_from_kfork(void);
extern void __attribute__((noreturn)) ret_from_execve(virtaddr_t entry, uint64_t rsp);

static const struct callee_regs default_regs = {
	0, 0, 0, 0, 0, 0, 0
};

#define TASK_KSTACK_ORDER 1
#define TASK_KSTACK_PAGES (1 << TASK_KSTACK_ORDER)
#define TASK_KSTACK_SIZE (TASK_KSTACK_PAGES * PAGE_SIZE)

static inline void copy_kernel_mappings(struct page_table *p4)
{
	memcpy(p4, kernel_mmu.p4, PAGE_SIZE);
}

static struct page_table *clone_pgtab(struct page_table *pgtab, size_t level)
{
	if (level == 0) {
		// We are copying a physical page, not a page table
		virtaddr_t dest = page_to_virt(pmm_alloc_order(0, GFP_NONE));
		virtaddr_t src = pgtab;
		kprintf("Copying %p to %p\n", src, dest);
		memcpy(dest, src, PAGE_SIZE);
		return dest;
	}

	struct page_table *rv = page_to_virt(pmm_alloc_order(0, GFP_NONE));
	size_t end_index = 512;
	if (level == 4) {
		copy_kernel_mappings(rv);
		end_index = (1 << 7);
	}

	for (size_t i = 0; i < end_index; i++) {
		if (pgtab->pages[i] & PAGE_PRESENT) {
			physaddr_t pgtab_phys = (physaddr_t)(pgtab->pages[i] & PTE_ADDR_MASK);
			virtaddr_t pgtab_virt = phys_to_virt(pgtab_phys);
			kassert_dbg(ISALIGN_POW2((uintptr_t)pgtab_virt, PAGE_SIZE));
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
	return t;
}

void __attribute__((noreturn)) task_execve(virtaddr_t function, char UNUSED(*argv[]), unsigned int UNUSED(flags))
{
	struct task *self = current();
	if (self->mmu == NULL) {
		self->mmu = kmalloc(sizeof(struct mmu_info), KM_NONE);
		self->mmu->p4 = page_to_virt(pmm_alloc_order(0, GFP_NONE));
		copy_kernel_mappings(self->mmu->p4);
		change_cr3(virt_to_phys(self->mmu->p4));
	} else {
		// TODO: Free all process-used low memory
		vmm_destroy_low_mappings(self->mmu);
	}

	// Set up the entry point
	uintptr_t entry = 0x1000;
	vmm_map_page(self->mmu, kern_to_phys(function), (virtaddr_t)entry, PAGE_EXECUTABLE | PAGE_USER_ACCESSIBLE);
	entry += ((uintptr_t)function & 0xFFF);

	for (size_t i = 0; i < 2; i++) {
		size_t off = i * PAGE_SIZE;
		uintptr_t page = (uintptr_t)page_to_virt(pmm_alloc_order(0, GFP_NONE));
		vmm_map_page(self->mmu, virt_to_phys((virtaddr_t)(page + off)),
				(virtaddr_t)(0x2000 + off), PAGE_WRITABLE | PAGE_USER_ACCESSIBLE);
	}

	ret_from_execve((virtaddr_t)entry, 0x4000);
}
