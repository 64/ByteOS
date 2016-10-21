#pragma once

#include <system.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory/kheap.h>
#include <sys/cdefs.h>
#include <memory/multiboot.h>

#define PAGE_SIZE 0x1000

#define PAGE_TABLE_PRESENT (1 << 0)
#define PAGE_TABLE_RW (1 << 1)
#define PAGE_TABLE_USER (1 << 2)
#define PAGE_TABLE_WRITETHROUGH (1 << 3)
#define PAGE_TABLE_FRAME(x) ((x) * PAGE_SIZE)

#define PAGE_DIR_PRESENT (1 << 0)
#define PAGE_DIR_RW (1 << 1)
#define PAGE_DIR_USER (1 << 2)
#define PAGE_DIR_WRITETHROUGH (1 << 3)

#define PAGE_FAULT_PRESENT (1 << 0)
#define PAGE_FAULT_RW (1 << 1)
#define PAGE_FAULT_USER (1 << 2)
#define PAGE_FAULT_RESERVED (1 << 3)
#define PAGE_FAULT_ID (1 << 4)

struct page_table {
	uint32_t pages[1024];
};

struct page_directory {
	struct page_table *tables[1024];
	uint32_t tables_physical[1024];
	uint32_t physical_addr;
};

extern struct page_directory *kernel_directory;
extern struct page_directory *current_directory;

void paging_init(multiboot_info_t *, uintptr_t, size_t);
void paging_change_dir(struct page_directory *);
uint32_t *paging_get(uintptr_t, bool, struct page_directory *);
void paging_fault(struct regs *);
void paging_alloc_frame(uint32_t *, bool, bool, uintptr_t);
void paging_free_frame(uintptr_t);
void paging_identity_map(uintptr_t address, size_t length, bool is_writeable, bool is_kernel);
void paging_generate_tables(uintptr_t address, size_t count, struct page_directory *dir);
