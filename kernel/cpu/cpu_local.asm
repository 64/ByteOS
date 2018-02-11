%include "include.asm"

section .text
; Initialises the per-CPU data area (GS register)
global cpu_local_init
cpu_local_init:
	mov rdi, SIZEOF_STRUCT_PERCPU
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

	mov qword [rdi], 0      ; task
	mov qword [rdi + 8], 0  ; rsp_scratch
	mov dword [rdi + 16], 0 ; id

	ret

global cpu_local_set_task
; rdi: Pointer to task struct
cpu_local_set_task:
	mov [gs:0x0], rdi
	extern tss64
	mov rdi, [rdi + SIZEOF_STRUCT_CONTEXT]
	mov [tss64 + 4], rdi ; RSP0
	ret
