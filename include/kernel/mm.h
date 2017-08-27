#pragma once

#include <stdint.h>
#include "multiboot2.h"
#include "system.h"

typedef uintptr_t physaddr_t;

void pmm_mmap_parse(struct multiboot_info *);

void *boot_heap_malloc(size_t n);
void boot_heap_free(void *p, size_t n);
void boot_heap_register_node(physaddr_t start_addr, size_t len);
