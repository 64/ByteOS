#include <stdio.h>

#include <drivers/ps2/mouse.h>
#include <drivers/ps2/kbd.h>
#include <drivers/ps2/ps2main.h>
#include <drivers/pit.h>
#include <drivers/acpi.h>
#include <memory/memory.h>
#include <memory/kheap.h>
#include <string.h>
#include <klog.h>
#include <asm.h>
#include <tty.h>

extern void gdt_install();
extern void idt_install();
extern void syscalls_install();

void kernel_early(uint32_t mboot_magic, const void *mboot_header) {
	// Initialises the screen so we can see what's going on
	vga_textmode_initialize();

	// Initialise interrupts, GDT and syscalls
	gdt_install();
	idt_install();
	syscalls_install();

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
	ps2_init();
	keyboard_init();
	mouse_init();
	klog_notice("PS/2 devices successfully initialized!\n");

	// Ensure kernel never exits
	while(1);
}
