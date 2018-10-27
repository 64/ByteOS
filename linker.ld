ENTRY(_start)
OUTPUT_FORMAT(elf64-x86-64)

phys = 0x100000;
offset = 0xFFFFFFFF80000000;
virt = offset + phys;

SECTIONS
{
        . = virt;

        .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - offset)
        {
                _rodata_begin_phys = . - offset;
                *(.multiboot_header)
                *(.rodata)
                *(.rodata*)
                *(.eh_frame)
                _rodata_end_phys = . - offset;
        }

        .text ALIGN(0x1000) : AT(ADDR(.text) - offset)
        {
                _text_begin_phys = . - offset;
                *(.text)
                *(.text*)
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
                _text_end_phys = . - offset;
        }

        .data ALIGN(0x1000) : AT(ADDR(.data) - offset)
        {
                _data_begin_phys = . - offset;
                *(.data)
                _data_end_phys = . - offset;
        }

        .bss ALIGN(0x1000) : AT(ADDR(.bss) - offset)
        {
                _bss_begin_phys = . - offset;
                *(.bss)
                *(COMMON)
                _bss_end_phys = . - offset;
        }

        /DISCARD/ :
        {
                *(.comment)
        }

        _kernel_end_phys = . + 0x1000 - offset;

}
