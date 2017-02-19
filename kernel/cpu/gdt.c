#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/util.h>
#include <descriptors.h>

static struct {
	struct gdt_entry entries[6]; //!< Contains the six entries to the GDT
	struct gdt_pointer pointer; //!< Stores the pointer which the CPU uses to locate the GDT
} gdt COMPILER_ATTR_USED;

void gdt_set_entry(uint8_t index, uint64_t base, uint64_t limit, uint8_t access, uint8_t granularity) {
	gdt.entries[index].base_low = (base & 0xFFFF);
	gdt.entries[index].base_middle = (base >> 16) & 0xFF;
	gdt.entries[index].base_high = (base >> 24) & 0xFF;
	gdt.entries[index].limit_low = (limit & 0xFFFF);
	gdt.entries[index].granularity = (limit >> 16) & 0xFF;
	gdt.entries[index].granularity |= (granularity & 0xF0);
	gdt.entries[index].access = access;
}

void gdt_install(void) {
	struct gdt_pointer *gdt_p = &gdt.pointer;
	gdt_p->size = sizeof(gdt.entries) - 1;
	gdt_p->base = (uintptr_t)&(gdt.entries[0]);

	gdt_set_entry(0, 0, 0, 0, 0);
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	gdt_load((uintptr_t)gdt_p);
}
