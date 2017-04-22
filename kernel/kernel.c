#include <stdio.h>

#include <drivers/ps2/mouse.h>
#include <drivers/ps2/kbd.h>
#include <drivers/ps2/ps2main.h>
#include <drivers/pit.h>
#include <drivers/acpi.h>
#include <memory/memory.h>
#include <memory/kheap.h>
#include <system/task.h>
#include <descriptors.h>
#include <string.h>
#include <klog.h>
#include <asm.h>
#include <tty.h>

extern uintptr_t stack_bottom;
extern void syscalls_install();

/// \brief Runs before `kernel_main` and initialises a few subsystems
/// \param mboot_header The multiboot magic value passed from the bootloader
/// \param mboot_magic The pointer to the multiboot struct passed from the bootloader
void kernel_early(uint32_t mboot_magic, const void *mboot_header) {
	// Initialises the screen so we can see what's going on
	vga_textmode_initialize();

	// Initialise interrupts, GDT, TSS and syscalls
	gdt_install();
	idt_install();
	tss_install(0x10, stack_bottom + 14384);
	syscalls_install();

	// ACPI must gather values before paging is enabled
	pit_init();
	acpi_init();

	// Validates multiboot, enables paging, sets up the heap
	mem_init(mboot_magic, mboot_header);

	// Enable SSE + other CPU extensions
	cpu_extensions_enable();
}

/// \brief Initialises device drivers and runs until shutdown
void kernel_main(void) {
	// Friendly greeting
	klog_info("Hello, Kernel World!\n");

	// Initialise subsystems
	ps2_init();
	keyboard_init();
	mouse_init();

	klog_notice("PS/2 devices successfully initialized!\n");

	//klog_notice("Launching scheduler...\n");
	//tasking_init();
	//void task_test(void); task_test();
	void enter_userspace(void); enter_userspace();

	// Ensure kernel never exits
	while(1);
}
