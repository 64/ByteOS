%include "include.asm"

bits 64

global spin_lock
spin_lock:
	preempt_inc
	mov rax, 1
	xchg [rdi], rax
	test rax, rax
	jz .acquired
.retry:
	pause
	bt qword [rdi], 0
	jc .retry

	xchg [rdi], rax
	test rax, rax
	jnz .retry
.acquired:
	ret

global spin_try_lock
spin_try_lock:
	preempt_inc
	mov rax, 1
	xchg [rdi], rax
	test rax, rax
	jz .acquired
	preempt_dec
	mov rax, 0
	ret
.acquired:
	mov rax, 1
	ret

global spin_unlock
spin_unlock:
	mov qword [rdi], 0
	preempt_dec
	ret
