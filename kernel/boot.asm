%define KERNEL_TEXT_BASE 0xFFFFFFFF80000000
%define KERNEL_PHYS_MAP_END 0x1000000
%define PAGE_SIZE 0x1000
section .multiboot_header
align 8 ; Must be 8 byte aligned as per the specification
header_start:
	dd 0xE85250D6		     ; Magic number (multiboot 2)
	dd 0			     ; Architecture 0 (protected mode i386)
	dd header_end - header_start ; Header length
	; Checksum
	dd 0x100000000 - (0xE85250D6 + 0 + (header_end - header_start))

	; Required end tag
	dw 0 ; Type
	dw 0 ; Flags
	dd 8 ; Size
header_end:

section .bss
global p4_table
align PAGE_SIZE
p4_table equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p3_table equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
lower_p3_table equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p2_table equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_0 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_1 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_2 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_3 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_4 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_5 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_6 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p1_table_7 equ $ - KERNEL_TEXT_BASE
	resb PAGE_SIZE
p3_table_temp equ $ - KERNEL_TEXT_BASE ; Use the top of the stack temporarily
stack_top:
	resb PAGE_SIZE * 4 ; 16kB
stack_bottom:

; Stacks for the Interrupt Stack Table
	resb PAGE_SIZE
ist_stack_1:
	resb PAGE_SIZE
ist_stack_2:

%macro map_page 3
	mov eax, %3
	or eax, 0b11 ; Writable, Present
	mov [%1 + %2 * 8], eax
	and dword [%1 + 4 + %2 * 8], ~(1 << 31) & 0xFFFFFFFF ; Clear NXE bit
%endmacro

; edx -> eax
%macro within_labels 2
	extern %1
	extern %2
	cmp edx, %1
	setnb al
	cmp edx, %2
	setb dl
	and eax, edx
%endmacro

section .data
global tss64
global interrupt_stack_table
tss64:
	dd 0
	times 3 dq 0 ; RSPn
	dq 0 ; Reserved
interrupt_stack_table:
	dq ist_stack_1 ; IST1, NMI
	dq ist_stack_2 ; IST2, Double fault
	dq 0 ; IST3
	dq 0 ; IST4
	dq 0 ; IST5
	dq 0 ; IST6
	dq 0 ; IST7
	dq 0 ; Reserved
	dw 0 ; Reserved
	dw 0 ; I/O Map Base Address
tss_size equ $ - tss64 - 1

global gdt64
gdt64:                               ; Global Descriptor Table (64-bit)
	.null equ $ - gdt64          ; The null descriptor
	dw 0                         ; Limit (low)
	dw 0                         ; Base (low)
	db 0                         ; Base (middle)
	db 0                         ; Access
	db 0                         ; Granularity
	db 0                         ; Base (high)
	.code equ $ - gdt64          ; The code descriptor
	dw 0                         ; Limit (low)
	dw 0                         ; Base (low)
	db 0                         ; Base (middle)
	db 10011010b                 ; Access (exec/read)
	db 00100000b                 ; Granularity
	db 0                         ; Base (high)
	.data equ $ - gdt64          ; The data descriptor
	dw 0                         ; Limit (low)
	dw 0                         ; Base (low)
	db 0                         ; Base (middle)
	db 10010010b                 ; Access (read/write)
	db 00000000b                 ; Granularity
	db 0                         ; Base (high)
	.user_code32 equ $ - gdt64   ; Ring 3 code descriptor (32-bit)
	dw 0                         ; Limit (low)
	dw 0                         ; Base (low)
	db 0                         ; Base (middle)
	db 0                         ; Access (exec/read)
	db 0                         ; Granularity
	db 0                         ; Base (high)
	.user_data equ $ - gdt64     ; Ring 3 data descriptor
	dw 0                         ; Limit (low)
	dw 0                         ; Base (low)
	db 0                         ; Base (middle)
	db 11110010b                 ; Access (read/write)
	db 00000000b                 ; Granularity
	db 0                         ; Base (high)
	.user_code64 equ $ - gdt64   ; Ring 3 code descriptor (64-bit)
	dw 0                         ; Limit (low)
	dw 0                         ; Base (low)
	db 0                         ; Base (middle)
	db 11111010b                 ; Access (exec/read)
	db 00100000b                 ; Granularity
	db 0                         ; Base (high)
	.tss equ $ - gdt64           ; The TSS descriptor
	dw tss_size & 0xFFFF         ; Limit
	dw 0                         ; Base (bytes 0-2)
	db 0                         ; Base (byte 3)
	db 10001001b                 ; Type, present
	db 00000000b                 ; Misc
	db 0                         ; Base (byte 4)
	dd 0                         ; Base (bytes 5-8)
	dd 0                         ; Zero / reserved
gdt_size equ $ - gdt64 - 1

section .text
bits 32
%define VGABUF 0xB8000
global _start
_start:
	; Disable interrupts
	cli

	; Clear the direction flag
	cld

	; Set stack pointer
	mov esp, stack_bottom - KERNEL_TEXT_BASE

	; Clear screen
	mov ecx, 2000 ; 80 * 25 characters in the buffer
	jmp .cls_end
.cls:
	dec ecx
	mov word [VGABUF + ecx * 2], 0x0F20
.cls_end:
	cmp ecx, 0
	jne .cls

	; Validate multiboot
	cmp eax, 0x36D76289
	jne .no_multiboot
	; Check 8-byte alignment of pointer
	mov ecx, ebx
	and ecx, (8 - 1)
	test ecx, ecx
	jnz .no_multiboot

	; Set the cursor to blink
	mov dx, 0x3D4
	mov al, 0x09
	out dx, al ; out 0x3D4, 0x09
	mov dx, 0x3D5
	mov al, 0x0F
	out dx, al ; out 0x3D5, 0x0F
	mov dx, 0x3D4
	mov al, 0x0B
	out dx, al ; out 0x3D4, 0x0B
	mov dx, 0x3D5
	mov al, 0x0F
	out dx, al ; out 0x3D5, 0x0F
	mov dx, 0x3D4
	mov al, 0x0A
	out dx, al ; out 0x3D4, 0x0A
	mov dx, 0x3D5
	mov al, 0x0E
	out dx, al ; out 0x3D5, 0x0E

	; Sets the cursor position to 0,1
	mov dx, 0x3D4
	mov al, 0x0F
	out dx, al ; out 0x3D4, 0x0F
	mov dx, 0x3D5
	mov al, 80 ; Set cursor position to (row * 80) + col = (1 * 80) + 0 = 80
	out dx, al ; out 0x3D5, 80
	mov dx, 0x3D4
	mov al, 0x0E
	out dx, al ; out 0x3D4, 0x0E
	mov dx, 0x3D5
	mov al, 0x00
	out dx, al ; out 0x3D5, 0x00

	; Check if CPUID is supported
	pushfd
	pop eax
	mov ecx, eax ; Save for later on
	xor eax, 1 << 21 ; Flip the ID bit
	push eax
	popfd
	; Put the old value back
	pushfd
	mov [esp], ecx
	popfd
	; See if it worked
	xor eax, ecx
	jz .no_cpuid

	; Preserve multiboot information
	push ebx

	; Check if long mode is supported
	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000001
	jb .no_long_mode

	mov eax, 0x80000001
	cpuid
	test edx, 1 << 29
	jz .no_long_mode

	; Pop multiboot information
	pop ebx

	; Enable SSE
	mov eax, cr0
	and eax, 0xFFFB
	or ax, 2
	mov cr0, eax
	mov eax, cr4
	or ax, 3 << 9
	mov cr4, eax

	; Setup paging
	map_page p4_table, 511, p3_table
	map_page p3_table, 510, p2_table
	map_page p2_table, 0, p1_table_0
	map_page p2_table, 1, p1_table_1
	map_page p2_table, 2, p1_table_2
	map_page p2_table, 3, p1_table_3
	map_page p2_table, 4, p1_table_4
	map_page p2_table, 5, p1_table_5
	map_page p2_table, 6, p1_table_6
	map_page p2_table, 7, p1_table_7

	; Map addresses from 0xFFFF800000000000
	map_page p4_table, 256, lower_p3_table
	map_page lower_p3_table, 0, p2_table

	; Map in some more memory around where we are so we have time to jump
	map_page p4_table, 0, p3_table_temp
	map_page p3_table_temp, 0, p2_table

	; Map each P1 entry from 0 - 2MB
	; Set permissions for each section
	; section .text:   r-x
	; section .rodata: r--
	; section .data:   rw-
	; section .bss:    rw-
	mov ecx, 0
.map_p1_table:
	mov eax, 4096
	mul ecx
	mov edi, eax
	mov esi, 0
	mov edx, edi
	within_labels _text_begin_phys, _text_end_phys
	test al, al
	jnz .is_text
	mov edx, edi
	within_labels _rodata_begin_phys, _rodata_end_phys
	test al, al
	jnz .is_rodata
.is_none: ; data, bss
	or edi, 1 << 1
	or esi, 1 << 31
	jmp .end
.is_text:
	and esi, ~(1 << 31) & 0xFFFFFFFF
	and edi, ~(1 << 1)
	jmp .end
.is_rodata: ; rodata
	and edi, ~(1 << 1)
	or esi, 1 << 31
.end:
	or edi, 0b100000001 ; Global, Present
	and edi, ~(1 << 2)  ; Supervisor only
	mov edx, p1_table_0
	mov [edx + ecx * 8], edi ; Low 4 bytes
	add edx, 4
	mov [edx + ecx * 8], esi ; High 4 bytes
	inc ecx
	cmp ecx, 512 * 8
	jne .map_p1_table

	; Make sure we allocated enough memory in the page tables for the kernel
	extern _kernel_end_phys
	mov eax, _kernel_end_phys
	cmp eax, KERNEL_PHYS_MAP_END
	jg .kernel_too_big

	; Enable paging
	; Set P4 address in CR3
	mov eax, p4_table
        mov cr3, eax
        ; Enable PAE flag
        mov eax, cr4
        or eax, 1 << 5
        mov cr4, eax
        ; Set long mode, NXE bit
        mov ecx, 0xC0000080
        rdmsr
        or eax, (1 << 8) | (1 << 11)
        wrmsr
        ; Enable paging, enable write protect mode
        mov eax, cr0
        or eax, (1 << 31) | (1 << 16)
	mov cr0, eax

	sub esp, 10
	mov word [esp], gdt_size
	mov dword [esp + 2], gdt64 - KERNEL_TEXT_BASE
	mov dword [esp + 6], 0x00000000

	lgdt [esp]
	jmp gdt64.code:long_mode

; Multiboot magic value was incorrect
.no_multiboot:
	mov al, "0"
	jmp .error

; Processor doesn't support CPUID instruction
.no_cpuid:
	mov al, "1"
	jmp .error

; Processor doesn't support long mode
.no_long_mode:
	mov al, "2"
	jmp .error

; Kernel executable file is greater than the memory we allocated for page tables
.kernel_too_big:
	mov al, "3"
	jmp .error

; .error: Writes "ERROR: $al" in red text and halts the CPU
; al: Contains the ASCII character to be printed to the screen
.error:
	mov dword [VGABUF], 0x4F524F45
	mov dword [VGABUF + 4], 0x4F4F4F52
	mov dword [VGABUF + 8], 0x4F3A4F52
	mov dword [VGABUF + 12], 0x4F204F20
	mov byte  [VGABUF + 14], al
	hlt

bits 64
long_mode equ $ - KERNEL_TEXT_BASE
	; Load TSS descriptor into GDT
	mov rdi, gdt64
	add rdi, gdt64.tss
	mov rax, tss64
	mov word [rdi + 2], ax
	shr rax, 16
	mov byte [rdi + 4], al
	shr rax, 8
	mov byte [rdi + 7], al
	shr rax, 8
	mov dword [rdi + 8], eax

	; Load GDT
	add rsp, KERNEL_TEXT_BASE
	mov qword [rsp + 2], gdt64
	lgdt [rsp]
	mov rax, higher_half
	jmp rax
higher_half:
	; Unmap the lower half page tables
	mov rax, p4_table
	add rax, KERNEL_TEXT_BASE
	mov qword [rax], 0
	mov rax, p3_table_temp
	add rax, KERNEL_TEXT_BASE
	mov qword [rax], 0

	; Flush whole TLB
	mov rax, cr3
	mov cr3, rax

	extern long_mode_start
	mov rax, long_mode_start
	jmp rax
