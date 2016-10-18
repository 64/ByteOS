#include <memory/paging.h>
#include <memory/kheap.h>
#include <stdlib.h>
#include <string.h>
#include <isr.h>
#include <klog.h>

uint32_t *frames;
uint32_t n_frames;

page_directory *kernel_directory = 0;
page_directory *current_directory = 0;

#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

void paging_init() {
	uintptr_t mem_end_page = 0x1000000;
	n_frames = mem_end_page / PAGE_SIZE;
	frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(n_frames));
	memset(frames, 0, INDEX_FROM_BIT(n_frames));

	kernel_directory = (page_directory*)kmalloc_a(sizeof(page_directory));
	memset(kernel_directory, 0, sizeof(page_directory));
	current_directory = kernel_directory;

	uint32_t i = 0;

	for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += PAGE_SIZE)
		paging_get(i, 1, kernel_directory);

	// TODO: This needs improving...
	paging_get(0x7FE0040, 1, kernel_directory); // QEMU Lower APIC information page
	paging_get(0x7FE18DC, 1, kernel_directory); // QEMU Higher APIC information page
	paging_get(0x3FF0000, 1, kernel_directory); // VirtualBox APIC information page
	paging_get(0x3FF1000, 1, kernel_directory); // VirtualBox APIC information page
	paging_get(0x3FF2000, 1, kernel_directory); // VirtualBox APIC information page

	i = 0;
	while (i < placement_address + PAGE_SIZE) {
		paging_alloc_frame(paging_get(i, 1, kernel_directory), 0, 0, 0);
		i += PAGE_SIZE;
	}

	paging_alloc_frame(paging_get(0x7FE0040, 0, kernel_directory), 0, 0, 0x7FE0040);
	paging_alloc_frame(paging_get(0x7FE18DC, 0, kernel_directory), 0, 0, 0x7FE18DC);
	paging_alloc_frame(paging_get(0x3FF0000, 0, kernel_directory), 0, 0, 0x3FF0000);
	paging_alloc_frame(paging_get(0x3FF1000, 0, kernel_directory), 0, 0, 0x3FF1000);
	paging_alloc_frame(paging_get(0x3FF2000, 0, kernel_directory), 0, 0, 0x3FF2000);

	for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += PAGE_SIZE)
		paging_alloc_frame(paging_get(i, 0, kernel_directory), 0, 0, 0);

	isr_install_handler(14, paging_fault);
	paging_change_dir(kernel_directory);
	kheap = kheap_create(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);
}

void paging_change_dir(page_directory *dir) {
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

uint32_t *paging_get(uintptr_t address, bool make, page_directory *dir) {
	address /= PAGE_SIZE;
	uint32_t table_index = address / 1024;
	if (dir->tables[table_index])
		return &dir->tables[table_index]->pages[address % 1024];
	else if (make) {
		uint32_t tmp;
		dir->tables[table_index] = (page_table*)kmalloc_ap(sizeof(page_table), &tmp);
		memset(dir->tables[table_index], 0, PAGE_SIZE);
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

	bool present = !(regs->err_code & 0x1);
	bool rw = regs->err_code & 0x2;
	bool us = regs->err_code & 0x4;
	bool reserved = regs->err_code & 0x8;
	uint8_t id = regs->err_code & 0x10;

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
        uint32_t idx = INDEX_FROM_BIT(frame);
        uint32_t off = OFFSET_FROM_BIT(frame);
        frames[idx] |= (0x1 << off);
}

static void paging_clear_frame(uintptr_t frame_addr) {
	uint32_t frame = frame_addr / PAGE_SIZE;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	frames[idx] &= ~(0x1 << off);
}

static uint32_t paging_test_frame(uintptr_t frame_addr) {
	uint32_t frame = frame_addr / PAGE_SIZE;
	uint32_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
	return (frames[idx] & (0x1 << off));
}

static uintptr_t paging_first_frame() {
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(n_frames); i++) {
		if (frames[i] != 0xFFFFFFFF) {
			for (j = 0; i < 32; j++) {
				uint32_t to_test = 0x1 << j;
				if (!(frames[i] & to_test))
					return (i * 4 * 8) + j;
			}
		}
	}
	klog_fatal("No free memory!");
	abort();
}

void paging_alloc_frame(uint32_t *page, bool is_kernel, bool is_writeable, uintptr_t phys) {
	if ((*page >> 12 & 0x000FFFFF) != 0)
		return;

	uintptr_t idx;
	if (phys == 0) {
		idx = paging_first_frame();
		if (idx == (uint32_t)-1)
			abort();
	} else
		idx = phys / PAGE_SIZE;

	paging_set_frame(idx * PAGE_SIZE);
	*page |= PAGE_TABLE_PRESENT;
	*page |= (is_writeable) ? PAGE_TABLE_RW : 0;
	*page |= (is_kernel) ? 0 : PAGE_TABLE_USER;
	*page |= PAGE_TABLE_FRAME(idx);
}

void paging_free_frame(uintptr_t page) {
	uint32_t frame;
	if ((frame = (((*(uint32_t*)page) >> 12 & 0x000FFFFF))) == 0)
		return;

	paging_clear_frame(frame);
	page |= PAGE_TABLE_FRAME(0);
}
