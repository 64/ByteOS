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
global cpu_local_init
cpu_local_init:
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
	pop rdi

	extern lapic_id
	call lapic_id

	mov qword [rdi + percpu.task], 0
	mov qword [rdi + percpu.rsp_scratch], 0
	mov byte [rdi + percpu.id], al

	ret

global cpu_local_set_task
; rdi: Pointer to task struct
cpu_local_set_task:
	mov [gs:0x0], rdi
	extern tss64
	mov rdi, [rdi + SIZEOF_STRUCT_CONTEXT]
	mov [tss64 + 4], rdi ; RSP0
	ret
