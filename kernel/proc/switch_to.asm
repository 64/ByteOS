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

	; Prepare to swap cr3
	mov rax, cr3
	mov rcx, [rdi + 0x10]

	; Check if the target cr3 is the kernel (we don't need to switch)
	extern kernel_mmu
	cmp rcx, kernel_mmu
	je .cr3_done

	; Check if the current cr3 is the same as the previous one
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
