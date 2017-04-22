#include <descriptors.h>
#include <system.h>
#include <string.h>

static struct tss_entry tss;

void tss_install(uint16_t ss0, uintptr_t esp0) {
	gdt_set_entry(5, (uintptr_t)&tss, sizeof(struct tss_entry), 0xE9, 0x00);

	memset(&tss, 0, sizeof(struct tss_entry));

	tss.ss0 = ss0;
	tss.esp0 = esp0;
	tss.cs = 0x0B;
	tss.ss = 0x13;
	tss.ds = 0x13;
	tss.es = 0x13;
	tss.fs = 0x13;
	tss.gs = 0x13;
	tss.iomap_base = sizeof(struct tss_entry);

	tss_load(); // Don't need to pass pointer to TSS, it's referenced by GDT offset
}

void tss_set_kernel_stack(uintptr_t esp) {
	tss.esp0 = esp;
}
