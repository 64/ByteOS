section .text
global context_switch
context_switch:
	; TODO: Is there a more concise way to express this in assembly?
	sub rsp, 64
	and rsp, ~15 ; Align to 16 bytes (is this necessary?)
	mov qword [rsp], 0 ; Error code

	; Copy the struct onto the stack
	mov rsi, rdi
	lea rdi, [rsp + 8]
	mov rcx, 5 * 8
	extern memcpy
	call memcpy
	iretq ; Execute the task switch
