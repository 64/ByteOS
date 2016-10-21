#pragma once

#include <stdint.h>
#include <stddef.h>

extern uintptr_t placement_address;

void mem_init(uint32_t multiboot_magic, const void *multiboot_header);

struct mem_info {
	uintptr_t lower;
	uintptr_t upper;
	int8_t *bootloader_name;
};

uintptr_t virt_to_phys(void *);
void *reserve_memory(uintptr_t physical, size_t length);
