#include <memory/paging.h>
#include <memory/kheap.h>
#include <memory/memory.h>
#include <algs/bitset.h>
#include <stdlib.h>
#include <string.h>
#include <interrupt.h>
#include <klog.h>

struct bitset paging_bitset;

struct page_directory *kernel_directory = 0;
struct page_directory *current_directory = 0;

#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

void paging_init(multiboot_info_t *mboot_hdr, uintptr_t mmap_end) {
	// TODO: This whole unit needs a big rewrite...
	kernel_directory = (struct page_directory*)kmalloc_a(sizeof(struct page_directory));
	memset(kernel_directory, 0, sizeof(struct page_directory));
	current_directory = kernel_directory;

	uint32_t i = 0;

	for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += PAGE_SIZE)
		paging_get(i, 1, kernel_directory);

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t*)mboot_hdr->mmap_addr;
	multiboot_memory_map_t *saved = mmap;
	size_t mem_upper_limit = 0;
	while((uintptr_t)mmap < mmap_end) {
		if (mmap->type != 1) {
			// Memory is reserved, so we should allocate pages for it.
			uintptr_t iter = mmap->addr_low; // Always 32-bit pointers
			while (iter <= mmap->addr_low + mmap->len_low) {
				paging_get(iter, 1, kernel_directory);
				iter += PAGE_SIZE;
			}
		}
		mem_upper_limit = mmap->addr_low + mmap->len_low;
		mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
	}

	// Allocate 1 bit for each page in available memory
	bitset_create(&paging_bitset, ((mem_upper_limit - 1) / PAGE_SIZE) / 8);

	i = 0;
	while (i < placement_address + PAGE_SIZE) {
		paging_alloc_frame(paging_get(i, 1, kernel_directory), 0, 0, 0);
		i += PAGE_SIZE;
	}

	mmap = saved;
	while((uintptr_t)mmap < mmap_end) {
		if (mmap->type != 1) {
			// Memory is reserved, so we should allocate pages for it.
			uintptr_t iter = mmap->addr_low; // Always 32-bit pointers
			while (iter <= mmap->addr_low + mmap->len_low) {
				paging_alloc_frame(paging_get(iter, 0, kernel_directory), 0, 0, iter);
				iter += PAGE_SIZE;
			}
		}
		mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
	}

	for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += PAGE_SIZE)
		paging_alloc_frame(paging_get(i, 1, kernel_directory), 0, 0, 0);

	isr_install_handler(14, paging_fault);
	paging_change_dir(kernel_directory);
	kheap = kheap_create(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);
	klog_detail("0x%x is 0x%x\n", virt_to_phys((void*)(KHEAP_START + PAGE_SIZE)), KHEAP_START + PAGE_SIZE);
}

void paging_change_dir(struct page_directory *dir) {
	current_directory = dir;
	asm volatile (
		"mov %0, %%cr3"
		:
		: "r" (dir->tables_physical)
	);
	uint32_t cr0;
	asm volatile (
		"mov %%cr0, %0"
		: "=r" (cr0)
		:
	);
	cr0 |= 0x80000000;
	asm volatile (
		"mov %0, %%cr0"
		:
		: "r" (cr0)
	);
}

uint32_t *paging_get(uintptr_t address, bool make, struct page_directory *dir) {
	address /= PAGE_SIZE;
	uint32_t table_index = address / 1024;
	if (dir->tables[table_index])
		return &dir->tables[table_index]->pages[address % 1024];
	else if (make) {
		uint32_t tmp;
		dir->tables[table_index] = (struct page_table*)kmalloc_ap(sizeof(struct page_table), &tmp);
		memset(dir->tables[table_index], 0, sizeof(struct page_table));
		dir->tables_physical[table_index] = tmp | 0x7;
		return &dir->tables[table_index]->pages[address % 1024];
	} else
		return 0;
}

void paging_fault(struct regs *regs) {
	uint32_t faulting_addr;
	asm volatile (
		"mov %%cr2, %0"
		: "=r" (faulting_addr)
		:
	);

	bool present = !(regs->err_code & PAGE_FAULT_PRESENT);
	bool rw = regs->err_code & PAGE_FAULT_RW;
	bool us = regs->err_code & PAGE_FAULT_USER;
	bool reserved = regs->err_code & PAGE_FAULT_RESERVED;
	uint8_t id = regs->err_code & PAGE_FAULT_ID;

	klog_fatal("Page fault: %x\n\t", faulting_addr);
	if (present) klog_fatal_nohdr("- Page not present\n\t");
	if (rw) klog_fatal_nohdr("- Page not writeable\n\t");
	if (us) klog_fatal_nohdr("- Page not writeable from user-mode\n\t");
	if (reserved) klog_fatal_nohdr("- Page reserved bits overwritten\n\t");
	if (id) klog_fatal_nohdr("- ID: %d\n", id);
	klog_fatal_nohdr("\n");
	abort();
}

static void paging_set_frame(uintptr_t frame_addr) {
	uint32_t frame = frame_addr / PAGE_SIZE;
        bitset_set(&paging_bitset, frame);
}

static void paging_clear_frame(uintptr_t frame_addr) {
	uint32_t frame = frame_addr / PAGE_SIZE;
	bitset_clear(&paging_bitset, frame);
}

static bool paging_test_frame(uintptr_t frame_addr) {
	uint32_t frame = frame_addr / PAGE_SIZE;
	return bitset_test(&paging_bitset, frame);
}

static uint32_t paging_first_frame() {
	// This function sets the value of 'out'
	size_t out = 0;
	if (bitset_find_first(&paging_bitset, &out) == 0) {
		klog_fatal("No free memory!\n");
		abort();
	}
	return out;
}

void paging_alloc_frame(uint32_t *page, bool is_kernel, bool is_writeable, uintptr_t phys) {
	if ((*page >> 12 & 0x000FFFFF) != 0)
		return;

	if (phys == 0) {
		uint32_t idx;
		idx = paging_first_frame();
		paging_set_frame(idx * PAGE_SIZE);
		*page |= PAGE_TABLE_FRAME(idx);
	} else {
		paging_set_frame(phys);
		*page |= PAGE_TABLE_FRAME(phys / PAGE_SIZE);
	}

	*page |= PAGE_TABLE_PRESENT;
	*page |= (is_writeable) ? PAGE_TABLE_RW : 0;
	*page |= (is_kernel) ? 0 : PAGE_TABLE_USER;
}

// Declared in <memory/memory.h>
uintptr_t virt_to_phys(void *virtual_addr) {
	uintptr_t v = (uintptr_t)virtual_addr;
	uintptr_t p = 0;
	uint32_t page_entry = *paging_get(v, 0, kernel_directory);
	if ((page_entry & PAGE_TABLE_PRESENT) == 0) {
		klog_warn("Virtual address lookup was out of range\n");
		return v;
	}
	p = page_entry & 0xFFFFF000;
	p += v & 0x00000FFF;
	return p;
}

void paging_free_frame(uintptr_t page) {
	uint32_t frame;
	if ((frame = (((*(uint32_t*)page) >> 12 & 0x000FFFFF))) == 0)
		return;

	paging_clear_frame(frame);
	page |= PAGE_TABLE_FRAME(0);
}
