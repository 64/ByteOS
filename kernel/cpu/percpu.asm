%include "include.asm"

struc percpu
	.task: resq 1
	.rsp_scratch: resq 1
	.id: resb 1
	resb 3
	resd 1 ; Padding
	.size:
endstruc

section .text
; Initialises the per-CPU data area (GS register)
global percpu_init
percpu_init:
	mov rdi, percpu.size
	mov rsi, 0
	extern kmalloc
	call kmalloc
	mov rdi, rax

	push rax
	mov ecx, 0xC0000101 ; IA32_GS_BASE MSR
	mov eax, edi
	shr rdi, 32
	mov edx, edi
	wrmsr

	extern lapic_id
	call lapic_id

	pop rdi
	mov qword [rdi + percpu.task], 0
	mov qword [rdi + percpu.rsp_scratch], 0
	mov byte [rdi + percpu.id], al

	ret
