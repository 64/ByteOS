#include <stdio.h>

#include <tty.h>
#include <memory.h>
#include <klog.h>
#include <io.h>
#include <drivers/ps2/kbd.h>
#include <drivers/pit.h>

void kernel_early(uint32_t mboot_magic, const void *mboot_header) {
	vga_textmode_initialize();
	mem_init(mboot_magic, mboot_header);
}

void kernel_main(void) {
	klog_info("Hello, Kernel World!\n");

	// Install main IRQ handlers
	pit_install();
	keyboard_install();

	// Ensure kernel never exits
	while(1);
}
