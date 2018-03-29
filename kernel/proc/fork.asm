%include "include.asm"

section .text
global ret_from_ufork
; Final work before returning to a child user space thread
ret_from_ufork:
	mov rax, 0
	iretq

global ret_from_kfork
; Final work before returning to a kernel thread
ret_from_kfork:
	pop rdi
	pop rax
	call rax
	mov rdi, [PERCPU_CURRENT]
	mov rsi, 0
	extern task_exit
	call task_exit
	; Doesn't return

global ret_from_execve
; Final work after an execve was called, returning to userspace
; rdi: Entry point of user task
; rsi: Stack pointer of user task
ret_from_execve:
	pushfq
	pop rax
	or rax, 0x200 ; Make sure interrupts are enabled

	push qword (GDT_USER_DATA | 0x3)
	push qword rsi
	push qword rax
	push qword (GDT_USER_CODE | 0x3)
	push qword rdi
	iretq
	
