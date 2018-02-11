%include "include.asm"

section .text
global task_switch_isr
%define save(reg, index) mov [rsp + 8 * (index)], reg
task_switch_isr:
	sub rsp, SIZEOF_STRUCT_CONTEXT
	; General purpose registers
	; TODO: These can be replaced by push instructions instead of mov
	save(rax, 0)
	save(rbx, 1)
	save(rcx, 2)
	save(rdx, 3)
	save(rdi, 4)
	save(rsi, 5)
	mov rax, [rsp + 24] ; rsp
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
	mov rax, [rsp + 24] ; rflags
	save(rax, 16)
	mov rax, [rsp] ; rip
	save(rax, 17)
	mov rax, [rsp + 8] ; cs
	save(rax, 18)
	mov rax, [rsp + 32] ; ss
	save(rax, 19)
	mov rax, cr3
	save(rax, 20)

	mov rdi, rsp
	extern schedule
	jmp schedule

global task_switch_fn
task_switch_fn:
	sub rsp, SIZEOF_STRUCT_CONTEXT
	; Save rflags and rax for some room
	pushf
	push qword rax
	mov rax, [rsp + 8]
	mov [rsp + (8 * 16) + 16], rax
	mov rax, [rsp]
	mov [rsp + 16], rax
	; Undo the last 2 pushes, rax and rflags are now saved
	add rsp, 16

	; Rest of the general purpose registers
	save(rbx, 1)
	save(rcx, 2)
	save(rdx, 3)
	save(rdi, 4)
	save(rsi, 5)
	lea rax, [rsp + SIZEOF_STRUCT_CONTEXT + 8] ; rsp (+8 is for return address)
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
	; rflags already done
	mov rax, [rsp + SIZEOF_STRUCT_CONTEXT] ; rip
	save(rax, 17)
	mov rax, 0x8 ; cs (guaranteed here to be from kernel)
	save(rax, 18)
	mov rax, 0x10 ; ss (guaranteed here to be from kernel)
	save(rax, 19)
	mov rax, cr3 ; PML4
	save(rax, 20)

	mov rdi, rsp
	extern schedule
	jmp schedule

global switch_to
%define restore(reg, index) mov reg, [rdi + 8 * (index)]
; rdi: Pointer to struct task
switch_to:
	push rdi
	extern cpu_local_set_task
	call cpu_local_set_task
	pop rdi

	; Setup the simulated interrupt frame
	push qword [rdi + 19 * 8] ; ss
	push qword [rdi + 6 * 8] ; rsp
	push qword [rdi + 16 * 8] ; rflags
	push qword [rdi + 18 * 8] ; cs
	push qword [rdi + 17 * 8] ; rip

	; If the current cr3 and target cr3 are the same, then don't swap them
	mov rax, cr3
	mov rbx, [rdi + 20 * 8]
	cmp rax, rbx
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
	iretq ; Execute the task switch
