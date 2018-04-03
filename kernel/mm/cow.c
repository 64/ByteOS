#include "mm.h"
#include "proc.h"
#include "percpu.h"
#include "libk.h"
#include "asm.h"

// Note: caller is expected to flush TLB and disable preemption
void cow_copy_pte(pte_t *dest, pte_t *src)
{
	struct page *page = phys_to_page(*src & PTE_ADDR_MASK);
	kref_inc(&page->refcount);

	// Make page read-only and CoW if it isn't already
	if (!(*src & PAGE_COW)) {
		*src &= ~PAGE_WRITABLE;
		*src |= PAGE_COW;
		kref_inc(&page->refcount);
	} else
		kassert_dbg(!(*src & PAGE_WRITABLE));

	// Copy the PTE
	*dest = *src;
}

// Returns true if the write is allowed and should be retried
bool cow_handle_write(pte_t *pte, virtaddr_t virt)
{
	if (pte == NULL || !(*pte & PAGE_COW))
		return false;

	struct page *page = phys_to_page(*pte & PTE_ADDR_MASK);
	spin_lock(&page->lock);

	//kprintf_nolock("Handle CoW write to address %p\n", (void *)page_to_phys(page));

	uint32_t next_count = kref_dec_read(&page->refcount);
	if (next_count > 0) {
		// We need to allocate another page, and copy the memory into it
		physaddr_t dest = page_to_phys(pmm_alloc_order(0, GFP_NONE));
		physaddr_t src = *pte & PTE_ADDR_MASK;
		memcpy(phys_to_virt(dest), phys_to_virt(src), PAGE_SIZE);

		// Write the new address into the PTE
		*pte &= ~PTE_ADDR_MASK;
		*pte |= (dest & PTE_ADDR_MASK);
	}

	// Otherwise, this was the last reference and we are free to reuse this memory
	spin_unlock(&page->lock);
	*pte &= ~PAGE_COW;
	*pte |= PAGE_WRITABLE;
	tlb_flush_single(virt);
	return true;
}

// TODO: This might be a race condition (either now or in the future) if another
// process tries to copy a PTE at the same time as we're freeing it.
void cow_handle_free(pte_t *pte)
{
	kassert_dbg(pte != NULL);
	kassert_dbg(*pte & PAGE_COW);

	physaddr_t phys = *pte & PTE_ADDR_MASK;
	if (phys < (uintptr_t)&_kernel_end_phys)
		return; // This page is probably mapped to the zero page or some kernel code

	struct page *page = phys_to_page(phys);

	uint64_t next_count = kref_dec_read(&page->refcount);
	if (next_count == 0) {
		pmm_free_order(page, 0);
	}
}
