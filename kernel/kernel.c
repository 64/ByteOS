#include <stdio.h>

#include <tty.h>
#include <memory.h>

char error_header[] = { '[', 27, '[', '3', '1', 'm', 'E', 'R', 'R', 'O', 'R', 27, '[', '0', 'm', ']', '\0' };
char info_header[] = { '[', 27, '[', '3', '6', 'm', 'I', 'N', 'F', 'O', 27, '[', '0', 'm', ']', '\0' };

void kernel_early(uint32_t mboot_magic, const void *mboot_header) {
	vga_textmode_initialize();
	mem_init(mboot_magic, mboot_header);
}

void kernel_main(void) {
	printf("%s Hello, Kernel World!\n", info_header);
}
