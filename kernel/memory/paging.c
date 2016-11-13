#include <stdlib.h>
#include <string.h>
#include <interrupt.h>
#include <klog.h>
#include <sys/util.h>
#include <memory/paging.h>
#include <memory/pmm.h>
#include <memory/kheap.h>

struct page_directory *kernel_directory = NULL;
struct page_directory *current_directory = NULL;

void paging_enable() {
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

void paging_change_dir(struct page_directory *dir) {
	current_directory = dir;
	asm volatile (
		"mov %0, %%cr3"
		:
		: "r" (dir->tables_physical)
	);
}

static void paging_parse_mmap_entry(phys_addr addr, size_t len, uint32_t type) {
	if (type != 1)
		pmm_reserve_block(addr, len);
}

void paging_init(multiboot_info_t *mboot_hdr, uintptr_t mmap_end, size_t available_max) {
	kernel_directory = (struct page_directory*)kmalloc_a(sizeof(struct page_directory));
	memset(kernel_directory, 0, sizeof(struct page_directory));
	current_directory = kernel_directory;
	pmm_init(available_max);

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t*)mboot_hdr->mmap_addr;
	while ((uintptr_t)mmap < mmap_end) {
		paging_parse_mmap_entry(mmap->addr_low, mmap->len_low, mmap->type);
		mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
	}

	paging_generate_tables(0, placement_address + PAGE_SIZE, kernel_directory);
	paging_generate_tables(KHEAP_START, KHEAP_MAX, kernel_directory);

	uint32_t i;
	for (i = 0; i <= placement_address; i += PAGE_SIZE)
		pmm_map_frame(i, i, 0);

	kheap_init();
	isr_install_handler(14, paging_fault);
	paging_change_dir(kernel_directory);
	paging_enable();
}

pt_entry *paging_get(virt_addr address, bool make, struct page_directory *dir) {
	if (dir == NULL)
		dir = current_directory;

	uint32_t page_offset = address / PAGE_SIZE;
	if (dir->tables[page_offset / 1024] != NULL)
		return &dir->tables[page_offset / 1024]->pages[page_offset % 1024];
	if (make) {
		paging_generate_tables(address, 0, kernel_directory);
		return &dir->tables[page_offset / 1024]->pages[page_offset % 1024];
	} else
		return NULL;
}

// Allocates page tables between two addresses, if end_addr is 0 then it will allocate only one table
void paging_generate_tables(virt_addr address, virt_addr end_addr, struct page_directory *dir) {
	phys_addr iter = address / PAGE_SIZE / 1024;
	while (iter < ((address / PAGE_SIZE / 1024) + ((end_addr / PAGE_SIZE / 1024) || 1))) {
		// If the table already exists
		if (dir->tables[iter] == NULL) {
			phys_addr physical_addr;
			dir->tables[iter] = (struct page_table*)kmalloc_ap(sizeof(struct page_table), &physical_addr);
			memset(dir->tables[iter], 0, sizeof(struct page_table));
			dir->tables_physical[iter] = physical_addr | PAGE_DIR_PRESENT | PAGE_DIR_RW | PAGE_DIR_USER;
		}
		iter++;
	}
}

// Declared in <system.h>
uintptr_t virt_to_phys(const virt_addr v) {
	uintptr_t p = 0;
	pt_entry page_entry = *paging_get(v, 0, kernel_directory);
	if ((page_entry & PAGE_TABLE_PRESENT) == 0) {
		klog_warn("Virtual address lookup was out of range\n");
		return v;
	}
	p = page_entry & 0xFFFFF000;
	p += v & 0x00000FFF;
	return p;
}

void paging_fault(struct interrupt_frame *regs) {
	uint32_t faulting_addr;
	asm(
		"mov %%cr2, %0"
		: "=r" (faulting_addr)
		:
	);

	bool present = !(regs->err_code & PAGE_FAULT_PRESENT);
	bool rw = regs->err_code & PAGE_FAULT_RW;
	bool us = regs->err_code & PAGE_FAULT_USER;
	bool reserved = regs->err_code & PAGE_FAULT_RESERVED;
	uint8_t id = regs->err_code & PAGE_FAULT_ID;

	klog_fatal("Page fault: 0x%x\n\t", faulting_addr);
	if (present) klog_fatal_nohdr("- Page not present\n\t");
	if (rw) klog_fatal_nohdr("- Page not writeable\n\t");
	if (us) klog_fatal_nohdr("- Page not writeable from user-mode\n\t");
	if (reserved) klog_fatal_nohdr("- Page reserved bits overwritten\n\t");
	if (id) klog_fatal_nohdr("- ID: %d\n\t", id);
	klog_fatal_nohdr("\n");
	abort();
}
