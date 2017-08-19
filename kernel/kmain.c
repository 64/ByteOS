#include "libk.h"
#include "multiboot2.h"
#include "system.h"

void kmain(void *mboot_info_physical) {
	char *mboot_info_virtual = (char *)mboot_info_physical + KERNEL_VIRTUAL_BASE;
	struct multiboot_info *mboot_info = (struct multiboot_info *)mboot_info_virtual;
	kprintf("Multiboot2 info address:  %p\n", mboot_info);
	kprintf("Total size: %u, reserved: %u", mboot_info->total_size, mboot_info->reserved);
}
