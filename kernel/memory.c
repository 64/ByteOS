#include <memory.h>
#include <multiboot.h>
#include <stdlib.h>
#include <stdio.h>

#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))

extern char error_header[];
extern char info_header[];

extern void gdt_install();
extern void idt_install();

void mem_init(uint32_t multiboot_magic, const void *multiboot_header) {

    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("%s Multiboot magic was not found!\n", error_header);
        abort();
    }

    // Initialise GDT/IDT
    gdt_install();
    idt_install();

    multiboot_info_t *header = (multiboot_info_t*)multiboot_header;

    if (CHECK_FLAG(header->flags, 1) == 0) {
        printf("%s Boot device is invalid\n", error_header);
        abort();
    }

    if (CHECK_FLAG(header->flags, 2) == 0) {
        printf("%s Command line: %s\n", info_header, header->cmdline);
        abort();
    }

    if (CHECK_FLAG(header->flags, 4) == 0 && CHECK_FLAG(header->flags, 5) == 0) {
        printf("%s Both bits 4 and 5 of the multiboot flags are set\n", error_header);
        abort();
    }

    if (CHECK_FLAG(header->flags, 6) == 0) {
        printf("%s Multiboot memory map is invalid\n", error_header);
        abort();
    }

}
