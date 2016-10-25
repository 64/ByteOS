#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <system.h>

void pmm_init(uintptr_t max_mem);
void pmm_alloc_frame(virt_addr addr, uint32_t page_flags);
void pmm_map_frame(virt_addr va, phys_addr pa, uint32_t page_flags);
void pmm_free_frame(virt_addr addr);
void pmm_reserve_block(phys_addr start, size_t length);
void pmm_unreserve_block(phys_addr start, size_t length);
