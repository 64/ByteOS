#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "system.h"

void kmain(void *mboot_info_physical);

void kmain(void *mboot_info_physical) {
	char *mboot_info_virtual = (char *)mboot_info_physical + KERNEL_TEXT_BASE;
	pmm_mmap_parse((struct multiboot_info *)mboot_info_virtual);
}
