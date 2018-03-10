%define PAGE_SIZE 0x1000

%define KERNEL_TEXT_BASE 0xFFFFFFFF80000000
%define KERNEL_PHYS_MAP_END 0x1000000

%define GDT_KERNEL_CODE 0x8
%define GDT_KERNEL_DATA 0x10
%define GDT_USER_DATA 0x20
%define GDT_USER_CODE 0x28
%define GDT_TSS 0x30

%define KERNEL_CPU_STACK_SIZE (PAGE_SIZE * 4)

%define NUM_SYSCALLS 3
%define ENOSYS 0xFFFFFFFFFFFFFFFF
%define SYSCALL_FORK 2

; outb(port, val)
%macro outb 2
	mov al, %2
	mov dx, %1
	out dx, al
%endmacro

