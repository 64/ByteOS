#include <stdio.h>

#include <tty.h>
#include <memory.h>
#include <io.h>

char error_header[] = { '[', 27, '[', '3', '1', 'm', 'E', 'R', 'R', 'O', 'R', 27, '[', '0', 'm', ']', '\0' };
char info_header[] = { '[', 27, '[', '3', '6', 'm', 'I', 'N', 'F', 'O', 27, '[', '0', 'm', ']', '\0' };

void kernel_early(uint32_t mboot_magic, const void *mboot_header) {
	vga_textmode_initialize();
	mem_init(mboot_magic, mboot_header);
}

void kernel_main(void) {
	printf("%s Hello, Kernel World!\n", info_header);
	char c = 0;
	while(1) {
		char new = io_inportb(0x60);
		if (new != c)
			printf("%c", (c = new));
	}
}
