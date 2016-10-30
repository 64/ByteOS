#include <stdio.h>

#include <tty.h>
#include <memory/memory.h>
#include <memory/kheap.h>
#include <string.h>
#include <klog.h>
#include <drivers/ps2/kbd.h>
#include <drivers/pit.h>
#include <drivers/acpi.h>
#include <asm.h>

void kernel_early(uint32_t mboot_magic, const void *mboot_header) {
	// Initialises the screen so we can see what's going on
	vga_textmode_initialize();

	// ACPI must gather values before paging is enabled
	pit_init();
	acpi_init();

	// Validates multiboot, enables paging, sets up the heap
	mem_init(mboot_magic, mboot_header);

	// Enable SSE + other CPU extensions
	cpu_extensions_enable();
}

void kernel_main(void) {
	klog_info("Hello, Kernel World!\n");

	// Initialise subsystems
	keyboard_init();

	int *a = kmalloc_a(12);
	int *b = kmalloc_a(12);
	klog_detail("a: 0x%x\n", a);
	klog_detail("b: 0x%x\n", b);

	// Ensure kernel never exits
	while(1);
}
