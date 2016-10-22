#include <memory/paging.h>
#include <memory/kheap.h>
#include <memory/memory.h>
#include <algs/bitset.h>
#include <stdlib.h>
#include <string.h>
#include <interrupt.h>
#include <klog.h>

struct bitset paging_bitset;
struct page_directory *kernel_directory = NULL;
struct page_directory *current_directory = NULL;

static bool paging_test_frame(uintptr_t addr);
static void paging_clear_frame(uintptr_t addr);
static void paging_set_frame(uintptr_t addr);
static uint32_t paging_first_frame();

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

void paging_init(multiboot_info_t *mboot_hdr, uintptr_t mmap_end, size_t available_max) {
	(void)mboot_hdr; (void)mmap_end;
	// Allocate 1 bit for each page in available memory
	bitset_create(&paging_bitset, (((available_max - 1) / PAGE_SIZE) / 8) + 1);
	kernel_directory = (struct page_directory*)kmalloc_a(sizeof(struct page_directory));
	memset(kernel_directory, 0, sizeof(struct page_directory));
	current_directory = kernel_directory;

	paging_identity_map(0, placement_address + KHEAP_INITIAL_SIZE, 0, 0);

	uint32_t i = 0;
	for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += PAGE_SIZE)
		paging_alloc_frame(i, 0, 0);

	kheap = kheap_create(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, KHEAP_START + KHEAP_MAX, 0, 0);
	isr_install_handler(14, paging_fault);
	paging_change_dir(kernel_directory);
	paging_enable();
}

uint32_t *paging_get(uintptr_t address, bool make, struct page_directory *dir) {
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
void paging_generate_tables(uintptr_t address, uintptr_t end_addr, struct page_directory *dir) {
	uint32_t iter = address / PAGE_SIZE / 1024;
	while (iter < ((address / PAGE_SIZE / 1024) + ((end_addr / PAGE_SIZE / 1024) || 1))) {
		// If the table already exists
		if (dir->tables[iter] == NULL) {
			uint32_t physical_addr;
			dir->tables[iter] = (struct page_table*)kmalloc_ap(sizeof(struct page_table), &physical_addr);
			memset(dir->tables[iter], 0, sizeof(struct page_table));
			dir->tables_physical[iter] = physical_addr | PAGE_DIR_PRESENT | PAGE_DIR_RW | PAGE_DIR_USER;
		}
		iter++;
	}
}

// Identity maps a block of memory, given a starting address and a length
void paging_identity_map(uintptr_t address, size_t length, bool is_writeable, bool is_kernel) {
	uintptr_t iter = 0;
	while (iter < length) {
		paging_idmap_frame(iter + address, is_writeable, is_kernel);
		iter += PAGE_SIZE;
	}
}

// Identity maps a single page
void paging_idmap_frame(uintptr_t address, bool is_writeable, bool is_kernel) {
	uint32_t *page_entry = paging_get(address, 1, kernel_directory);
	paging_set_frame(address);
	*page_entry |= PAGE_TABLE_PRESENT;
	*page_entry |= (is_writeable) ? PAGE_TABLE_RW : 0;
	*page_entry |= (is_kernel) ? 0 : PAGE_TABLE_USER;
	*page_entry |= PAGE_TABLE_FRAME(address / PAGE_SIZE);
}

// Allocates a frame at the next available physical page
void paging_alloc_frame(uintptr_t address, bool is_writeable, bool is_kernel) {
	uint32_t *page_entry = paging_get(address, 1, kernel_directory);
	uint32_t next_frame = paging_first_frame();
	paging_set_frame(next_frame * PAGE_SIZE);
	*page_entry |= PAGE_TABLE_PRESENT;
	*page_entry |= (is_writeable) ? PAGE_TABLE_RW : 0;
	*page_entry |= (is_kernel) ? 0 : PAGE_TABLE_USER;
	*page_entry |= PAGE_TABLE_FRAME(next_frame);
}

// Declared in <memory/memory.h>
uintptr_t virt_to_phys(const void *virtual_addr) {
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

	klog_fatal("Page fault: 0x%x\n\t", faulting_addr);
	if (present) klog_fatal_nohdr("- Page not present\n\t");
	if (rw) klog_fatal_nohdr("- Page not writeable\n\t");
	if (us) klog_fatal_nohdr("- Page not writeable from user-mode\n\t");
	if (reserved) klog_fatal_nohdr("- Page reserved bits overwritten\n\t");
	if (id) klog_fatal_nohdr("- ID: %d\n\t", id);
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
