%include "include.asm"

section .data
global idt64
idt64:
	times 65 dq 0, 0
.pointer:
	dw $ - idt64 - 1
	dq idt64

section .text
isr_common:
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
	extern isr_handler
	call isr_handler
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rax
	add rsp, 16 ; rsp points to rip before iretq
	iretq

global load_idt
load_idt:
	jmp .isr_install
.isr_post_install:
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

	mov rax, idt64.pointer ; Is this needed?
	lidt [rax]
	ret

.isr_install_single:
	mov rdi, idt64
	lea rdi, [rdi + rdx]
	mov word [rdi], ax
	mov word [rdi + 2], 0x08
	mov byte [rdi + 4], sil ; IST index
	mov byte [rdi + 5], 0x8E
	shr rax, 16
	mov word [rdi + 6], ax
	shr rax, 16
	mov dword [rdi + 8], eax
	mov dword [rdi + 12], 0x00000000
	ret

%macro isr_addentry_ist 2
	mov rax, interrupt_service_routines.isr_%1
	mov rdx, %1 * 16
	mov sil, %2
	call .isr_install_single
%endmacro

%macro isr_addentry 1
	isr_addentry_ist %1, 0
%endmacro


.isr_install:
	isr_addentry 0
	isr_addentry 1
	isr_addentry_ist 2, 1 ; NMI
	isr_addentry 3
	isr_addentry 4
	isr_addentry 5
	isr_addentry 6
	isr_addentry 7
	isr_addentry_ist 8, 2 ; Double fault
	isr_addentry 9
	isr_addentry 10
	isr_addentry 11
	isr_addentry 12
	isr_addentry 13
	isr_addentry 14
	isr_addentry 15
	isr_addentry 16
	isr_addentry 17
	isr_addentry 18
	isr_addentry 19
	isr_addentry 20
	isr_addentry 21
	isr_addentry 22
	isr_addentry 23
	isr_addentry 24
	isr_addentry 25
	isr_addentry 26
	isr_addentry 27
	isr_addentry 28
	isr_addentry 29
	isr_addentry 30
	isr_addentry 31
	isr_addentry 32
	isr_addentry 33
	isr_addentry 34
	isr_addentry 35
	isr_addentry 36
	isr_addentry 37
	isr_addentry 38
	isr_addentry 39
	isr_addentry 40
	isr_addentry 41
	isr_addentry 42
	isr_addentry 43
	isr_addentry 44
	isr_addentry 45
	isr_addentry 46
	isr_addentry 47
	isr_addentry 47
	isr_addentry 48
	isr_addentry 49
	isr_addentry 50
	isr_addentry 51
	isr_addentry 52
	isr_addentry 53
	isr_addentry 54
	isr_addentry 55
	isr_addentry 56
	isr_addentry 57
	isr_addentry 58
	isr_addentry 59
	isr_addentry 60
	isr_addentry 61
	isr_addentry 62
	isr_addentry 63

	; Setup task switch interrupt
	extern task_switch_isr
	mov rax, task_switch_isr
	mov rdx, 64 * 16
	mov sil, 0
	call .isr_install_single

	jmp .isr_post_install

%macro isr_noerr 1
.isr_%1:
	nop
	push qword 0
	push qword %1
	jmp isr_common
%endmacro
%macro isr_err 1
.isr_%1:
	nop
	push qword %1
	jmp isr_common
%endmacro

interrupt_service_routines:
	isr_noerr 0
	isr_noerr 1
	isr_noerr 2
	isr_noerr 3
	isr_noerr 4
	isr_noerr 5
	isr_noerr 6
	isr_noerr 7
	isr_err 8
	isr_noerr 9
	isr_err 10
	isr_err 11
	isr_err 12
	isr_err 13
	isr_err 14
	isr_noerr 15
	isr_noerr 16
	isr_err 17
	isr_noerr 18
	isr_noerr 19
	isr_noerr 20
	isr_noerr 21
	isr_noerr 22
	isr_noerr 23
	isr_noerr 24
	isr_noerr 25
	isr_noerr 26
	isr_noerr 27
	isr_noerr 28
	isr_noerr 29
	isr_err 30
	isr_noerr 31

	; IRQs 0-15
	isr_noerr 32
	isr_noerr 33
	isr_noerr 34
	isr_noerr 35
	isr_noerr 36
	isr_noerr 37
	isr_noerr 38
	isr_noerr 39
	isr_noerr 40
	isr_noerr 41
	isr_noerr 42
	isr_noerr 43
	isr_noerr 44
	isr_noerr 45
	isr_noerr 46
	isr_noerr 47

	; Extra stuff
	isr_noerr 48
	isr_noerr 49
	isr_noerr 50
	isr_noerr 51
	isr_noerr 52
	isr_noerr 53
	isr_noerr 54
	isr_noerr 55
	isr_noerr 56
	isr_noerr 57
	isr_noerr 58
	isr_noerr 59
	isr_noerr 60
	isr_noerr 61
	isr_noerr 62
	isr_noerr 63
