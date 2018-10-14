%include "include.asm"

section .text
global flush_gdt_tss
flush_gdt_tss:
	; rdi: gdt
	; rsi: GDT_SIZE
	; rdx: tss

	sub rsp, 10
	mov word [rsp], si
	mov qword [rsp + 2], rdi

	lgdt [rsp]
	mov ax, GDT_TSS
	ltr ax

	add rsp, 10
	ret


