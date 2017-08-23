#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "system.h"

void kmain(void *mboot_info_physical) {
	char *mboot_info_virtual = (char *)mboot_info_physical + KERNEL_VIRTUAL_BASE;
	pmm_mmap_parse((struct multiboot_info *)mboot_info_virtual);

	for (size_t i = 0; i < 3; i++) {
		int *x = boot_heap_malloc(sizeof(int));
		kprintf("Allocated at: %p\n", (void*)x);
	}

	void print_heap(void);
	print_heap();
}
