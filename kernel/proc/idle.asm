%include "include.asm"
bits 64

section .rodata
msg: db 'Idle', 0xA, 0

section .text
global idle_task
idle_task:
.loop:
	extern kprintf
	mov rdi, msg
	xor rax, rax
	call kprintf
	sti
	hlt
	jmp .loop
