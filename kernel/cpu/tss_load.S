.section .text
.align 4

.global tss_load
.type tss_load, @function

tss_load:
	mov $0x2B, %ax
	ltr %ax
	ret
