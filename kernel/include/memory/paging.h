#pragma once

#include <system.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <system.h>
#include <memory/kheap.h>
#include <memory/multiboot.h>

#define PAGE_TABLE_PRESENT (1 << 0)
#define PAGE_TABLE_RW (1 << 1)
#define PAGE_TABLE_USER (1 << 2)
#define PAGE_TABLE_WRITETHROUGH (1 << 3)
#define PAGE_INTERNAL_GENTABLES (1 << 4)
#define PAGE_TABLE_ADDR(x) ((x) & 0xFFFFF000)

#define PAGE_DIR_PRESENT (1 << 0)
#define PAGE_DIR_RW (1 << 1)
#define PAGE_DIR_USER (1 << 2)
#define PAGE_DIR_WRITETHROUGH (1 << 3)

#define PAGE_FAULT_PRESENT (1 << 0)
#define PAGE_FAULT_RW (1 << 1)
#define PAGE_FAULT_USER (1 << 2)
#define PAGE_FAULT_RESERVED (1 << 3)
#define PAGE_FAULT_ID (1 << 4)

typedef uint32_t pt_entry;

struct page_table {
	pt_entry pages[1024];
};

struct page_directory {
	struct page_table *tables[1024];
	phys_addr tables_physical[1024];
	phys_addr physical_addr;
};

extern struct page_directory *kernel_directory;
extern struct page_directory *current_directory;

void paging_init(multiboot_info_t *, uintptr_t, size_t);
void paging_change_dir(struct page_directory *dir);
pt_entry *paging_get(virt_addr, bool, struct page_directory *);
void paging_fault(struct interrupt_frame *r);
void paging_generate_tables(phys_addr, size_t length, struct page_directory *);
