#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "system.h"

void kmain(void *mboot_info_physical);

void kmain(void *mboot_info_physical) {
	paging_init();
	struct multiboot_info *mboot_info_virtual = phys_to_kern((physaddr_t)mboot_info_physical);
	struct mmap *mem_map = mmap_init(mboot_info_virtual);
	paging_map_all(mem_map);
	mmap_dump_info();
}
