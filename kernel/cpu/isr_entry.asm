%include "include.asm"

section .data
global idt64
idt64:
	times 256 dq 0, 0
.pointer:
	dw $ - idt64 - 1
	dq idt64

section .text
global load_idt
load_idt:
	; Remap the PIC
	outb 0x20, 0x11
	outb 0xA0, 0x11
	outb 0x21, 0x20
	outb 0xA1, 0x28
	outb 0x21, 0x04
	outb 0xA1, 0x02
	outb 0x21, 0x01
	outb 0xA1, 0x01
	outb 0x21, 0x00
	outb 0xA1, 0x00

	; Disable the PIC (mask all interrupts)
	outb 0xA1, 0xFF
	outb 0x21, 0xFF

	extern idt_init
	call idt_init

	mov rax, idt64.pointer ; Is this needed?
	lidt [rax]
	ret

%macro isr_call_fn 1
	push rax
	push rcx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	lea rdi, [rsp + 72] ; Pointer to interrupt frame
	extern %1
	call %1
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
	add rsp, 8 ; rsp points to rip before iretq
	iretq
%endmacro

global isr_noop
isr_noop:
	nop
	iretq

global isr_irq
isr_irq:
	nop
	push qword 0
	isr_call_fn irq_handler

global isr_irq_noop
isr_irq_noop:
	nop
	extern lapic_base
	mov dword [lapic_base + 0xB0], 0
	iretq

exception_routine:
	isr_call_fn exception_handler

%macro isr_exception 1
global isr_exception_%1
isr_exception_%1:
	nop
	; Store the interrupt number in the highest four bytes of the error code
	; This way we can always increment rsp by 8 before iretq and no memory is wasted.
	mov dword [rsp + 4], %1
	jmp exception_routine
%endmacro

isr_exception 0
isr_exception 1
isr_exception 2
isr_exception 3
isr_exception 4
isr_exception 5
isr_exception 6
isr_exception 7
isr_exception 8
isr_exception 9
isr_exception 10
isr_exception 11
isr_exception 12
isr_exception 13
isr_exception 14
isr_exception 15
isr_exception 16
isr_exception 17
isr_exception 18
isr_exception 19
isr_exception 20
isr_exception 21
isr_exception 22
isr_exception 23
isr_exception 24
isr_exception 25
isr_exception 26
isr_exception 27
isr_exception 28
isr_exception 29
isr_exception 30
isr_exception 31
