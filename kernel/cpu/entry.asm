%include "include.asm"

section .text
global long_mode_entry
long_mode_entry:
	; Long mode doesn't care about most segments.
	; GS and FS base addresses can be set with MSRs.
	mov ax, 0
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Multiboot structure (physical address)
	push rbx

	; Initialise VGA textmode driver
	extern vga_tmode_init
	call vga_tmode_init

	; Load interrupt descriptor table
	extern interrupts_init
	call interrupts_init

	; Load TSS
	mov ax, GDT_TSS
	ltr ax

	; Enable SSE, AVX, AVX-512
	extern simd_init
	call simd_init

	; Enable syscall/sysret instruction
	extern syscall_enable
	call syscall_enable

	; Call global constructors
	extern _init
	call _init

	; Pass multiboot information to kmain
	pop rdi
	extern kmain
	call kmain

	; Don't call global destructors - we should never get here
	sti
.end:
	hlt
	jmp .end

