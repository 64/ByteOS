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
	mov rsi, [PERCPU_CURRENT]
	mov [PERCPU_CURRENT], rdi

	; Set RSP0 in TSS
	mov rax, [PERCPU_TSS] 
	mov rcx, [rdi + 0x8]
	mov [rax + 4], rcx

	; Set the target MMU
	mov rdi, [rdi + 0x10] ; next
	mov rsi, [rsi + 0x10] ; previous
	extern mmu_switch
	call mmu_switch

	; Re-enable preemption
	lock dec qword [PERCPU_PREEMPT_COUNT]

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	ret
