#include <memory/kheap.h>

extern uint32_t end;
uintptr_t placement_address = (uintptr_t)&end;

uintptr_t kmalloc_internal(uint32_t size, bool align, uint32_t *phys) {
	if (align == 1 && (placement_address & 0xFFFFF000)) {
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}

	if (phys != NULL)
		*phys = placement_address;

	uintptr_t temp = placement_address;
	placement_address += size;
	return temp;
}

uintptr_t kmalloc_a(uint32_t size) {
	return kmalloc_internal(size, 1, NULL);
}

uintptr_t kmalloc_p(uint32_t size, uint32_t *phys) {
	return kmalloc_internal(size, 0, phys);
}

uintptr_t kmalloc_ap(uint32_t size, uint32_t *phys) {
	return kmalloc_internal(size, 1, phys);
}

uintptr_t kmalloc(uint32_t size) {
	return kmalloc_internal(size, 0, NULL);
}
