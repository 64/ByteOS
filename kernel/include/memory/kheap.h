#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern uintptr_t placement_address;

uintptr_t kmalloc_internal(uint32_t size, bool align, uint32_t *phys); // Internal use only.
uintptr_t kmalloc_a(uint32_t size);  // Page aligned.
uintptr_t kmalloc_p(uint32_t size, uint32_t *phys); // Returns a physical address.
uintptr_t kmalloc_ap(uint32_t size, uint32_t *phys); // Page aligned and returns a physical address.
uintptr_t kmalloc(uint32_t size); // Vanilla (normal).
