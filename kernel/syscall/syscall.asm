%include "include.asm"

section .rodata
no_syscall_message:
	db 0x1B, '[41mFatal error: syscall/sysret are not supported', 0xA, 0 ; Newline, NUL
	
section .text
; Enables the 'syscall' and 'sysret' instructions
global syscall_enable
syscall_enable:
	; Check that syscall and sysret are actually available
	mov eax, 0x80000001
	xor ecx, ecx
	cpuid
	test edx, (1 << 11)
	jz .no_syscall

	; IA32_STAR MSR
	mov ecx, 0xC0000081
	rdmsr
	; Load user 32-bit cs into STAR[63:48] and load kernel cs into STAR[47:32]
	mov edx, 0x00180008
	wrmsr

	; IA32_LSTAR MSR
	mov ecx, 0xC0000082
	rdmsr
	; Load rip into LSTAR
	mov rdi, syscall_entry
	mov eax, edi
	shr rdi, 32
	mov edx, edi
	wrmsr

	; Enable both instructions
	mov ecx, 0xC0000080 ; IA32_EFER MSR
	rdmsr
	or eax, (1 << 0)
	wrmsr

	; Set FMASK MSR for rflags
	mov ecx, 0xC0000084 ; IA32_FMASK MSR
	rdmsr
	or eax, (1 << 9) ; Disable interrupts upon syscall entry
	wrmsr

	ret
.no_syscall:
	mov rdi, no_syscall_message
	xor rax, rax ; Needed for varargs
	extern kprintf
	call kprintf
	extern abort
	jmp abort

; Entry point for the 'syscall' instruction
syscall_entry:
	; rip is in rcx
	; rflags is in r11

	; Switch to kernel stack
	mov [gs:0x8], rsp
	mov rsp, [gs:0x0]
	mov rsp, [0] ; TODO: Load kernel's stack pointer

	; Setup simulated IRQ frame
	push qword (GDT_USER_DATA | 0x3)
	push qword [gs:0x8] ; Old RSP
	pushfq ; rflags
	push qword (GDT_USER_CODE | 0x3)
	push rcx ; rip

	; Interrupts will be safely handled on the kernel stack
	sti

	push rax ; info

	; Was the syscall out of range?
	cmp rax, NUM_SYSCALLS
	jae .bad_syscall

	extern syscall_table
	mov rax, [syscall_table + rax * 8]

	; Save registers
	push rdi
	push rsi
	push rdx
	push qword 0 ; rcx
	push qword ENOSYS ; rax
	push r8
	push r9
	push r10
	push qword 0 ; r11
	call rax
	add rsp, 8 ; r11
	pop r10
	pop r9
	pop r8
	add rsp, 16 ; rax, rcx
	pop rdx
	pop rsi
	pop rdi

	jmp .done
.bad_syscall:
	mov rax, ENOSYS
.done:
	add rsp, 8 ; info
	pop rcx ; rip
	add rsp, 8 ; cs
	pop r11 ; rflags

	; Disable interrupts (don't want interrupts using a ring 3 stack)
	cli

	; Switch to user stack again
	pop rsp ; rsp
	add rsp, 8 ; ss

	; rip is in rcx
	; rflags is in r11
	o64 sysret
