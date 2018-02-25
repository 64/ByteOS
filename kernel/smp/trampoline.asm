; This all gets copied into low memory before booting an AP
%include "include.asm"

%define LOAD_ADDR 0x1000
%define TMP_DATA_OFFSET 0x0A00
%define DATA_OFFSET (LOAD_ADDR + TMP_DATA_OFFSET + ($ - tmp_data_virt))
%define LABEL_OFFSET(label) (LOAD_ADDR + ((label) - smp_trampoline_start))

align PAGE_SIZE
global smp_trampoline_start
smp_trampoline_start equ $

; Don't put anything here, since the entry point needs to be page aligned

bits 16
smp_trampoline_16:
	cli
	lidt [tmp_idt_ptr_null]
	jmp 0x0:LABEL_OFFSET(.fix_cs)
.fix_cs:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	cld

	lgdt [tmp_gdt_ptr]

	; Enable PAE, global
	mov eax, cr4
	or eax, (1 << 5) | (1 << 7)
	mov cr4, eax

	; Jump to long mode
	extern p4_table
	mov eax, p4_table
	mov cr3, eax

	mov ecx, 0xC0000080 ; EFER
	rdmsr
	or eax, (1 << 8) | (1 << 11) ; Long mode, NX
	wrmsr

	; Enable paging, pmode, write protect
	mov eax, (1 << 0) | (1 << 31) | (1 << 16)
	mov cr0, eax

	jmp 0x08:LABEL_OFFSET(smp_trampoline_64)

bits 64
smp_trampoline_64:
	lea rax, [tmp_gdt_ptr]

	extern gdt_size
	extern gdt64
	mov word [tmp_gdt_ptr], gdt_size
	mov qword [tmp_gdt_ptr + 2], gdt64
	lgdt [tmp_gdt_ptr]

	; Restore it to its previous state for the next AP
	mov word [tmp_gdt_ptr], (tmp_gdt_end - tmp_gdt - 1)
	mov qword [tmp_gdt_ptr + 2], tmp_gdt

	jmp far dword [LABEL_OFFSET(.far_ptr)]
.far_ptr:
	dd LABEL_OFFSET(.fix_cs)
	dw 0x08
.fix_cs:

	extern smp_ap_stack
	mov rsp, [smp_ap_stack]

	extern smp_ap_started_flag
	mov byte [smp_ap_started_flag], 1

	extern load_idt
	mov rax, load_idt
	call rax

	extern smp_ap_kmain
	mov rax, smp_ap_kmain
	call rax

	sti
.end:
	hlt
	jmp .end

times TMP_DATA_OFFSET - ($ - smp_trampoline_start) db 0
tmp_data_virt:
tmp_gdt equ DATA_OFFSET
	; NULL selector
	dq 0
	; 64-bit kernel code
	dw 0xFFFF
	dw 0
	db 0
	db 10011010b
	db 10101111b
	db 0
	; 64-bit kernel data
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 10101111b
	db 0
tmp_gdt_end equ DATA_OFFSET

align 16
tmp_gdt_ptr equ DATA_OFFSET
	dw (tmp_gdt_end - tmp_gdt - 1)
	dq tmp_gdt

; Any NMI will shutdown, which is what we want
tmp_idt_ptr_null equ DATA_OFFSET
	dw 0
	dq 0

global smp_trampoline_end
smp_trampoline_end equ $
