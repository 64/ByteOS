#pragma once

#include <stdint.h>
#include "system.h"

typedef uintptr_t physaddr_t
typedef void* virtaddr_t

static inline virtaddr_t phys_to_virt(physaddr_t p) {
	return (virtaddr_t)(p + KERNEL_VIRTUAL_BASE);
}
