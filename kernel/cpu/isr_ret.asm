bits 64

section .text
global ret_from_interrupt:
ret_from_interrupt:
	; TODO: Check preempt_count
	; Check need_reschedule flag
	mov al, [gs:0x10]
	test al, al
	jnz .need_reschedule

.done:
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

.need_reschedule:
	extern schedule
	call schedule
	jmp .done
