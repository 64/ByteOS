#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "system.h"

void kmain(void *mboot_info_physical);

void kmain(void *mboot_info_physical) {
	paging_init();
	struct multiboot_info *mboot_info_virtual = phys_to_kern((physaddr_t)mboot_info_physical);
	mmap_init(mboot_info_virtual);
	kprintf("Random alloc: %p\n", (void *)mmap_alloc_low(2 * PAGE_SIZE).base);
	mmap_dump_info();
	// TODO: Update highest_mapped in mmap
}
