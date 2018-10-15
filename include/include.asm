%include "gen/syscall_gen.asm"

%define PAGE_SIZE 0x1000

%define KERNEL_TEXT_BASE 0xFFFFFFFF80000000
%define KERNEL_PHYS_MAP_END 0x1000000

%define GDT_KERNEL_CODE 0x8
%define GDT_KERNEL_DATA 0x10
%define GDT_USER_DATA 0x20
%define GDT_USER_CODE 0x28
%define GDT_TSS 0x30 ; Change this in percpu.c too

%define KERNEL_CPU_STACK_SIZE (PAGE_SIZE * 4)

%define PERCPU_CURRENT gs:0x0
%define PERCPU_RSP_SCRATCH gs:0x8
%define PERCPU_TSS gs:0x10
%define PERCPU_PREEMPT_COUNT gs:0x18
%define PERCPU_ID gs:0x1C
%define PERCPU_RUN_QUEUE gs:0x20

; outb(port, val)
%macro outb 2
	mov al, %2
	mov dx, %1
	out dx, al
%endmacro

%macro preempt_inc 0
	lock inc dword [gs:0x18]
%endmacro

%macro preempt_dec 0
	lock dec dword [gs:0x18]
%endmacro

%macro bochs_magic 0
	xchg bx, bx
%endmacro

%macro debug_hlt 0
.debug_hlt:
	cli
	hlt
	jmp .debug_hlt
%endmacro

