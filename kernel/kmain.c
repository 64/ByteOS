#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "system.h"

void kmain(void *mboot_info_physical);

void kmain(void *mboot_info_physical) {
	paging_init();
	char *mboot_info_virtual = phys_to_kern((physaddr_t)mboot_info_physical);
	pmm_mmap_parse((struct multiboot_info *)mboot_info_virtual);
}
