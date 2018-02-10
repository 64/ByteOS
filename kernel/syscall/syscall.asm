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
	mov edx, 0x1808
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

	; TODO: Maybe set FMASK MSR for rflags?
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

	; Important: switch to kernel stack

	; Execute syscall code
	hlt ; TODO: Implement

	; Important: switch to user stack

	; rip is in rcx
	; rflags is in r11
	sysret
