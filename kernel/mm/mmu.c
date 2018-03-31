#include "mm.h"
#include "util.h"
#include "libk.h"
#include "proc.h"
#include "percpu.h"

static inline void copy_kernel_mappings(struct page_table *p4)
{
	memcpy(p4, kernel_mmu.p4, PAGE_SIZE);
}

struct mmu_info *mmu_alloc(void)
{
	struct mmu_info *rv = kmalloc(sizeof(struct mmu_info), KM_NONE);
	memset(rv, 0, sizeof *rv);
	mmu_inc_users(rv);
	return rv;
}

void mmu_free(struct mmu_info *mmu)
{
	kassert_dbg(mmu != &kernel_mmu);
	kassert_dbg(kref_read(&mmu->users) == 0);
	mmu_reap(mmu);
	kfree(mmu);
}

void mmu_init(struct mmu_info *mmu)
{
	mmu->p4 = page_to_virt(pmm_alloc_order(0, GFP_NONE));
	copy_kernel_mappings(mmu->p4);
}

void mmu_switch_to(struct mmu_info *mmu)
{
	struct mmu_info *current_mmu = current->mmu;
	if (current_mmu != mmu) {
		spin_lock(&current_mmu->cpu_lock);
		cpuset_set_id(&current_mmu->cpus, percpu_get(id), 0);
		spin_unlock(&current_mmu->cpu_lock);

		spin_lock(&mmu->cpu_lock);
		cpuset_set_id(&mmu->cpus, percpu_get(id), 1);
		spin_unlock(&mmu->cpu_lock);
	}

	write_cr3(virt_to_phys(mmu->p4));
}

void mmu_inc_users(struct mmu_info *mmu)
{
	if (mmu != &kernel_mmu) {
		kref_inc(&mmu->users);
	}
}

void mmu_dec_users(struct mmu_info *mmu)
{
	if (mmu != &kernel_mmu) {
		uint32_t num = kref_dec_read(&mmu->users);
		if (num == 0)
			mmu_free(mmu);
	}
}

void mmu_reap(struct mmu_info *mmu)
{
	kassert_dbg(mmu != &kernel_mmu);

	// Only loop over the userspace mappings
	for (size_t p4_index = 0; p4_index < (1 << 7); p4_index++) {
		struct page_table *p3 = pgtab_extract_virt_addr(mmu->p4, p4_index);
		if (p3 == NULL)
			continue;
		for (size_t p3_index = 0; p3_index < 512; p3_index++) {
			struct page_table *p2 = pgtab_extract_virt_addr(p3, p3_index);
			if (p2 == NULL)
				continue;
			for (size_t p2_index = 0; p2_index < 512; p2_index++) {
				struct page_table *p1 = pgtab_extract_virt_addr(p2, p2_index);
				if (p1 == NULL)
					continue;
				for (size_t p1_index = 0; p1_index < 512; p1_index++) {
					if (p1->pages[p1_index] & PAGE_PRESENT) {
						if (p1->pages[p1_index] & PAGE_COW) {
							cow_handle_free(&p1->pages[p1_index]);
						} else {
							physaddr_t phys = p1->pages[p1_index] & PTE_ADDR_MASK;
							pmm_free_order(phys_to_page(phys), 0);
						}
						p1->pages[p1_index] = 0;
					}
				}
				pmm_free_order(virt_to_page(p1), 0);
				p2->pages[p2_index] = 0;
			}
			pmm_free_order(virt_to_page(p2), 0);
			p3->pages[p3_index] = 0;
		}
		pmm_free_order(virt_to_page(p3), 0);
		mmu->p4->pages[p4_index] = 0;
	}

	// Prevent freeing the PML4 from underneath us
	mmu_switch_to(&kernel_mmu); 

	free_page_at(mmu->p4);
	mmu->p4 = NULL;

	// TODO: Might need a TLB shootdown here.
}

static inline pte_t clone_single_page(pte_t *pte)
{
	pte_t dest;
	cow_copy_pte(&dest, pte);
	return dest;
}

static struct page_table *clone_pgtab(struct page_table *pgtab, size_t level)
{
	struct page_table *rv = page_to_virt(pmm_alloc_order(0, GFP_NONE));
	size_t end_index = 512;
	memset(rv, 0, sizeof *rv);

	if (level == 4) {
		copy_kernel_mappings(rv);
		end_index = (1 << 7);
	}

	for (size_t i = 0; i < end_index; i++) {
		if (pgtab->pages[i] & PAGE_PRESENT) {
			if (level == 1) {
				rv->pages[i] = clone_single_page(&pgtab->pages[i]);
			} else {
				uint64_t flags = pgtab->pages[i] & ~PTE_ADDR_MASK;
				physaddr_t pgtab_phys = (physaddr_t)(pgtab->pages[i] & PTE_ADDR_MASK);
				virtaddr_t pgtab_virt = phys_to_virt(pgtab_phys);
				kassert_dbg(ISALIGN_POW2((uintptr_t)pgtab_virt, PAGE_SIZE));
				rv->pages[i] = virt_to_phys(clone_pgtab(pgtab_virt, level - 1)) | flags;
			}
		}
	}

	return rv;
}

void mmu_clone_cow(struct mmu_info *dest, struct mmu_info *mmu)
{
	// Copy mappings from the parent
	kassert_dbg(dest->p4 == NULL);
	dest->p4 = clone_pgtab(mmu->p4, 4);

	// Since the entire address space got mapped as read only, we need to invalidate all of it
	uintptr_t cr3 = read_cr3();
	if (cr3 == (uintptr_t)mmu->p4)
		write_cr3(cr3);
}
