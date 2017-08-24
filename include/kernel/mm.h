#pragma once

#include <stdint.h>
#include "multiboot2.h"
#include "system.h"

void pmm_mmap_parse(struct multiboot_info *);

void boot_heap_init(void);
void *boot_heap_malloc(size_t n);
void boot_heap_free(void *p);
