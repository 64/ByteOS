%include "include.asm"
bits 64

section .text
global ret_and_reschedule
global ret_direct
ret_and_reschedule:
	; If NEED_PREEMPT = 1 and preempt_count = 0, then reschedule
	mov rax, [PERCPU_CURRENT]
	mov rax, [rax + 0x18] ; rax = task->flags
	bt rax, 1 ; TODO: Why does "bt [rax + 0x20], 1" not work?
	jc need_reschedule
ret_direct:
	pop r11
	pop r10
	pop r9
	pop r8
	pop rax
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	add rsp, 8 ; Info field
	iretq
need_reschedule:
	bochs_magic
	mov rax, [PERCPU_PREEMPT_COUNT]
	test rax, rax
	jnz ret_direct
	extern schedule
	call schedule
	jmp ret_direct
