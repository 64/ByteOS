%include "include.asm"

bits 64
section .text
; rdi: Pointer to next task
global switch_to
switch_to:
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	; Save current rsp
	mov rax, [PERCPU_CURRENT]
	mov [rax], rsp
	; Load next rsp
	mov rsp, [rdi]

	; Set current in per-cpu data
	mov [PERCPU_CURRENT], rdi

	; Set RSP0 in TSS
	mov rax, [PERCPU_TSS] 
	mov rcx, [rdi + 0x8]
	mov [rax + 4], rcx

	; Swap cr3 if necessary
	mov rax, cr3
	mov rcx, [rdi + 0x10]
	test rcx, rcx
	jz .cr3_done
	mov rcx, [rcx]
	mov rsi, 0xFFFF800000000000
	sub rcx, rsi
	cmp rax, rcx
	je .cr3_done
	mov rax, rcx
	mov cr3, rax

.cr3_done:
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	ret
