section .text
; Enables the 'syscall' and 'sysret' instructions
global syscall_enable
syscall_enable:
	; TODO: Do we need to check that syscall and sysret are available?
	; IA32_STAR MSR
	mov ecx, 0 ; TODO: Find correct value
	rdmsr
	; Load user 32-bit cs into STAR[63:48] and load kernel cs into STAR[47:32]
	mov edx, 0x1808
	wrmsr

	; IA32_LSTAR MSR
	mov ecx, 0 ; TODO: Find correct value
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

; Entry point for the 'syscall' instruction
syscall_entry:
	; rip is in rcx
	; rflags is in r11

	; Switch to kernel stack
	
	; Execute syscall code

	; Switch to user stack

	; rip is in rcx
	; rflags is in r11
	sysret
