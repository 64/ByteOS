.section .text
.align 4

.macro ISR_ERR_HANDLER index, has_error=0
	.global _isr\index
	_isr\index:
		cli
		.ifeq \has_error
			push $0
		.endif
		push $\index
		jmp isr_common
.endm

.macro IRQ_HANDLER index byte
	.global _irq\index
	.type _irq\index, @function
	_irq\index:
		cli
		push $0
		push $\byte
		jmp irq_common
.endm

ISR_ERR_HANDLER 0
ISR_ERR_HANDLER 1
ISR_ERR_HANDLER 2
ISR_ERR_HANDLER 3
ISR_ERR_HANDLER 4
ISR_ERR_HANDLER 5
ISR_ERR_HANDLER 6
ISR_ERR_HANDLER 7
ISR_ERR_HANDLER 8,  1
ISR_ERR_HANDLER 9
ISR_ERR_HANDLER 10, 1
ISR_ERR_HANDLER 11, 1
ISR_ERR_HANDLER 12, 1
ISR_ERR_HANDLER 13, 1
ISR_ERR_HANDLER 14, 1
ISR_ERR_HANDLER 15
ISR_ERR_HANDLER 16
ISR_ERR_HANDLER 17, 1
ISR_ERR_HANDLER 18
ISR_ERR_HANDLER 19
ISR_ERR_HANDLER 20
ISR_ERR_HANDLER 21
ISR_ERR_HANDLER 22
ISR_ERR_HANDLER 23
ISR_ERR_HANDLER 24
ISR_ERR_HANDLER 25
ISR_ERR_HANDLER 26
ISR_ERR_HANDLER 27
ISR_ERR_HANDLER 28
ISR_ERR_HANDLER 29
ISR_ERR_HANDLER 30, 1
ISR_ERR_HANDLER 31

IRQ_HANDLER 0, 32
IRQ_HANDLER 1, 33
IRQ_HANDLER 2, 34
IRQ_HANDLER 3, 35
IRQ_HANDLER 4, 36
IRQ_HANDLER 5, 37
IRQ_HANDLER 6, 38
IRQ_HANDLER 7, 39
IRQ_HANDLER 8, 40
IRQ_HANDLER 9, 41
IRQ_HANDLER 10, 42
IRQ_HANDLER 11, 43
IRQ_HANDLER 12, 44
IRQ_HANDLER 13, 45
IRQ_HANDLER 14, 46
IRQ_HANDLER 15, 47

.extern isr_fault_handler
.type isr_fault_handler, @function

.extern irq_handler
.type irq_handler, @function

isr_common:
	pusha

	push %ds
	push %es
	push %fs
	push %gs
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	push %esp
	call isr_fault_handler
	add $4, %esp

	pop %gs
	pop %fs
	pop %es
	pop %ds

	popa
	add $8, %esp
	iret

irq_common:
	pusha

	push %ds
	push %es
	push %fs
	push %gs
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	push %esp
	call irq_handler
	add $4, %esp

	pop %gs
	pop %fs
	pop %es
	pop %ds

	popa
	add $8, %esp
	iret
