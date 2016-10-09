#pragma once

#include <stdint.h>
#include <stddef.h>

extern uintptr_t placement_address;

void mem_init(uint32_t multiboot_magic, const void *multiboot_header);

typedef struct {
	uintptr_t lower;
	uintptr_t upper;
	int8_t *bootloader_name;
} memory_information;
