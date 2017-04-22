#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 0x1000

struct interrupt_frame {
	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, unused, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, esp, ss;
};

struct registers {
	uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
};

typedef uintptr_t phys_addr;
typedef uintptr_t virt_addr;

phys_addr virt_to_phys(const virt_addr v);
