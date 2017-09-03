#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "system.h"

void kmain(physaddr_t mboot_info_physical) {
	paging_init();
	char *mboot_info_virtual = (char *)phys_to_kern(mboot_info_physical);
	pmm_mmap_parse((struct multiboot_info *)mboot_info_virtual);
}
