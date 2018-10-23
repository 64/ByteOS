#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot2.h"
#include "smp.h"
#include "sync.h"
#include "atomic.h"
#include "mm_types.h"
#include "ds/linked.h"

#define KERNEL_PAGE_DATA (KERNEL_TEXT_BASE + KERNEL_PHYS_MAP_END)

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER_ACCESSIBLE (1ULL << 2)
#define PAGE_WRITE_THROUGH (1ULL << 3)
#define PAGE_DISABLE_CACHE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_COW (1ULL << 9)
#define PAGE_EXECUTABLE (1ULL << 63)

#define VMM_ALLOC_MMAP (1 << 15)
#define VMM_UNMAP (1 << 16)

#define PAGE_SHIFT 12
#define PTE_ADDR_MASK (~(0xFFF0000000000FFFUL))

#define MMAP_ALLOC_PA (1 << 0)
#define MMAP_MAX_REGIONS 128

#define GFP_CAN_FAIL (1 << 0)

#define GFP_NONE 0
#define VMM_NONE 0
#define KM_NONE 0


extern const uint8_t zero_page[PAGE_SIZE];
extern struct mmu_info kernel_mmu;
extern const uintptr_t _kernel_end_phys;

extern struct tlb_op * volatile tlb_op_location;
extern atomic32_t tlb_remaining_cpus;

void vmm_init(void);
void vmm_map_all(struct mmap *);
bool vmm_has_flags(struct mmu_info *, void *, uint64_t flags);
pte_t *vmm_get_pte(struct mmu_info *, const void *);
void vmm_map_page(struct mmu_info *, physaddr_t, virtaddr_t, unsigned long);
void vmm_dump_tables(struct mmu_info *);

struct mmap *mmap_init(struct multiboot_info *);
void mmap_dump_info(void);
struct mmap_region mmap_alloc_low(size_t n, unsigned int alloc_flags);

void pmm_init(struct mmap *);
struct page *pmm_alloc_order(unsigned int order, unsigned int alloc_flags) __attribute__((warn_unused_result));
void pmm_free_order(struct page *page, unsigned int order);

#define get_free_page() page_to_virt(pmm_alloc_order(0, GFP_NONE))
#define free_page_at(addr) pmm_free_order(virt_to_page(addr), 0)

void *kmalloc(size_t, unsigned int) __attribute__((malloc));
void kfree(void *);

void cow_copy_pte(pte_t *dest, pte_t *src);
bool cow_handle_write(pte_t *pte, virtaddr_t virt);
void cow_handle_free(pte_t *pte);

struct mmu_info *mmu_alloc(void);
void mmu_free(struct mmu_info *mmu);
void mmu_init(struct mmu_info *mmu);
void mmu_switch(struct mmu_info *next, struct mmu_info *prev);
void mmu_inc_users(struct mmu_info *mmu);
void mmu_dec_users(struct mmu_info *mmu);
void mmu_update_cpuset(struct mmu_info *mmu, cpuid_t id, bool val);
void mmu_reap(struct mmu_info *mmu);
void mmu_clone_cow(struct mmu_info *dest, struct mmu_info *mmu);

void area_add(struct mmu_info *mmu, struct vm_area *area);

void tlb_shootdown(struct mmu_info *mmu, virtaddr_t start, virtaddr_t end);

static inline void tlb_flush_single(virtaddr_t addr)
{
	invlpg((uintptr_t)addr);
}

static inline void tlb_flush_all(void)
{
	write_cr3(read_cr3());
}

static inline struct page_table *pgtab_extract_virt_addr(struct page_table *pgtab, uint16_t index)
{
	pte_t entry = pgtab->pages[index];
	if ((entry & PAGE_PRESENT) == 0)
		return NULL;
	return phys_to_virt((entry & PTE_ADDR_MASK));
}
