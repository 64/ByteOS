section .text
%define save(reg, index) mov [rdi + 8 * (index)], reg
global save_context
; rdi: pointer to struct context
; rsi: pointer to interrupt stack frame
; rdi and rsi must have been saved by the caller already
; Be careful not to clobber any other registers
save_context:
	; General purpose registers
	save(rax, 0)
	save(rbx, 1)
	save(rcx, 2)
	save(rdx, 3)
	; rdi saved by caller
	; rsi saved by caller
	mov rax, [rsi + 32] ; rsp
	save(rax, 6)
	save(rbp, 7)
	save(r8, 8)
	save(r9, 9)
	save(r10, 10)
	save(r11, 11)
	save(r12, 12)
	save(r13, 13)
	save(r14, 14)
	save(r15, 15)
	mov rax, [rsi + 24] ; rflags
	save(rax, 16)
	mov rax, [rsi + 8] ; rip
	save(rax, 17)
	mov rax, [rsi + 16] ; cs
	save(rax, 18)
	mov rax, [rsi + 40] ; ss
	save(rax, 19)
	mov rax, cr3
	mov [rdi + 20 * 8], rax
	ret

global restore_context
%define restore(reg, index) mov reg, [rdi + 8 * (index)]
; rdi: Pointer to struct context
restore_context:
	; If the current cr3 and target cr3 are the same, then don't swap (might be unnecessary)
	mov rax, cr3
	mov rbx, [rdi + 20 * 8]
	test rax, rbx
	je .restore
	mov cr3, rbx ; Change PML4 (flush entire TLB, very slow)
.restore:
	restore(rax, 0)
	restore(rbx, 1)
	restore(rcx, 2)
	restore(rdx, 3)
	; rdi done last
	restore(rsi, 5)
	; rsp done via iretq
	restore(rbp, 7)
	restore(r8, 8)
	restore(r9, 9)
	restore(r10, 10)
	restore(r11, 11)
	restore(r12, 12)
	restore(r13, 13)
	restore(r14, 14)
	restore(r15, 15)
	; rflags done via iretq
	; rip done via iretq
	; cs done via iretq
	; ss done via iretq
	; cr3 already done
	restore(rdi, 4)
	ret

global context_switch
context_switch:
	sub rsp, 64
	and rsp, ~15 ; Align to 16 bytes (is this necessary?)
	mov qword [rsp], 0 ; Error code

	; Copy the struct onto the stack
	; TODO: Is there a more concise way to express this in assembly?
	mov rax, [rdi + 17 * 8] ; rip
	mov [rsp + 8],  rax
	mov rax, [rdi + 18 * 8] ; cs
	mov [rsp + 16], rax
	mov rax, [rdi + 16 * 8] ; rflags
	mov [rsp + 24], rax
	mov rax, [rdi + 6 * 8] ; rsp
	mov [rsp + 32], rax
	mov rax, [rdi + 19 * 8] ; ss
	mov [rsp + 40], rax

	call restore_context

	iretq ; Execute the task switch
