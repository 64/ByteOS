.section .text
.align 4

.global gdt_load
.type gdt_load, @function

gdt_load:
	mov 4(%esp), %eax
	lgdt (%eax)

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	ljmp $0x08, $_gdt_flush

_gdt_flush:
	ret
