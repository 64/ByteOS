#include "libk.h"
#include "multiboot2.h"
#include "mm.h"
#include "system.h"

void kmain(void *mboot_info_physical) {
	char *mboot_info_virtual = (char *)mboot_info_physical + KERNEL_TEXT_BASE;
	pmm_mmap_parse((struct multiboot_info *)mboot_info_virtual);

	void boot_heap_print(void);
	int *x = boot_heap_malloc(4);
	int *y = boot_heap_malloc(4);
	int *z = boot_heap_malloc(4);
	boot_heap_free(x);
	boot_heap_free(y);
	boot_heap_free(z);
	boot_heap_print();
}
