#include <memory/memory.h>
#include <memory/multiboot.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <klog.h>
#include <memory/paging.h>

#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))

extern void gdt_install();
extern void idt_install();

struct mem_info mem_info;

void mem_init(uint32_t multiboot_magic, const void *multiboot_header) {
	if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		klog_fatal("Multiboot magic was not found!\n");
		abort();
	}

	// Initialise GDT/IDT
	gdt_install();
	idt_install();

	multiboot_info_t *header = (multiboot_info_t*)multiboot_header;

	if (CHECK_FLAG(header->flags, 1) == 0) {
		klog_fatal("Boot device is invalid\n");
		abort();
	}

	if (CHECK_FLAG(header->flags, 2) == 0) {
		klog_notice("Command line: %s\n", header->cmdline);
		abort();
	}

	if (CHECK_FLAG(header->flags, 4) == 0 && CHECK_FLAG(header->flags, 5) == 0) {
		klog_fatal("Both bits 4 and 5 of the multiboot flags are set\n");
		abort();
	}

	if (CHECK_FLAG(header->flags, 6) == 0) {
		klog_fatal("Multiboot memory map is invalid\n");
		abort();
	}

	memset(&mem_info, 0, sizeof(struct mem_info));
	mem_info.lower = header->mem_lower;
	mem_info.upper = header->mem_upper;
	mem_info.bootloader_name = (int8_t *)header->boot_loader_name;

	klog_detail("Memory map address: %x\n", header->mmap_addr);
	klog_detail("Memory map length: %d\n", header->mmap_length);

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t*)header->mmap_addr;
	for (mmap = (multiboot_memory_map_t *)header->mmap_addr; (uint32_t)mmap < header->mmap_addr + header->mmap_length;
		mmap = (multiboot_memory_map_t *)((uint32_t) mmap + mmap->size + sizeof (mmap->size))) {
		klog_detail("Memory map entry: \n");
		klog_detail_nohdr("\tsize = 0x%x, base_addr = 0x%x%x, ", (uint32_t)mmap->size, mmap->addr >> 32, mmap->addr & 0xFFFFFFFF);
		klog_detail_nohdr("length = 0x%x%x, type = 0x%x\n", mmap->len >> 32, mmap->len & 0xFFFFFFFF, (uint32_t)mmap->type);
	}

	paging_init();
}
