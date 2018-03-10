%include "include.asm"

global ret_from_ufork
; Final work before returning to a child user space thread
ret_from_ufork:
	mov rax, 0
	iretq

global ret_from_kfork
; Final work before returning to a kernel thread
ret_from_kfork:
	ret
