%include "include.asm"

section .text
global long_mode_start
long_mode_start:
	mov ax, GDT_USER_DATA
	mov ss, ax
	mov ax, 0
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push rbx ; Multiboot structure (physical address)

	; Enable SSE3+, AVX
	call enable_simd

	; Call global constructors
	extern _init
	call _init

	; Initialise VGA textmode driver
	extern vga_tmode_init
	call vga_tmode_init

	; Load interrupt descriptor table
	extern load_idt
	call load_idt

	; Load TSS
	mov ax, GDT_TSS
	ltr ax

	; Enable syscall/sysret instruction
	extern syscall_enable
	call syscall_enable

	; Initialise the APIC
	extern apic_init
	call apic_init

	; Pass multiboot information to kmain
	pop rdi
	extern kmain
	call kmain

	; Call global destructors
	extern _fini
	call _fini

	sti
.end:
	hlt
	jmp .end

; Enables AVX and AVX-512 if available
; TODO: Test this
enable_simd:
	push rax
	push rbx
	push rcx
	push rdx

	; AVX
	mov eax, 1
	xor rcx, rcx
	cpuid
	and ecx, (1 << 26) ; Detect XSAVE
	test ecx, ecx
	jz .done
	mov eax, 1
	xor rcx, rcx
	cpuid
	and ecx, (1 << 28) ; Detect AVX
	test ecx, ecx
	jz .done
	
	; Enable it
	xor ecx, ecx
	xgetbv
	or eax, 7
	xsetbv

	; AVX-512
	mov eax, 0x0D
	xor rcx, rcx
	cpuid
	and ecx, 0xE0 ; Detect AVX-512
	cmp ecx, 0xE0
	jne .done
	
	; Enable it
	xor ecx, ecx
	xgetbv
	or eax, 0xE0
	xsetbv
.done:
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
