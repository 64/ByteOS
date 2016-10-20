#include <stdio.h>

#include <tty.h>
#include <memory/memory.h>
#include <string.h>
#include <klog.h>
#include <drivers/ps2/kbd.h>
#include <drivers/pit.h>
#include <drivers/acpi.h>
#include <drivers/cmos.h>

void kernel_early(uint32_t mboot_magic, const void *mboot_header) {
	vga_textmode_initialize();
	mem_init(mboot_magic, mboot_header);
}

void kernel_main(void) {
	klog_info("Hello, Kernel World!\n");

	// Initialise subsystems
	pit_init();
	acpi_init();
	keyboard_init();

	// Ensure kernel never exits
	while(1);
}
