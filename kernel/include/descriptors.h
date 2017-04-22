#pragma once

#include <stdint.h>
#include <sys/util.h>
#include <system.h>

struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} COMPILER_ATTR_PACKED;

struct gdt_pointer {
	uint16_t size;
	uintptr_t base;
} COMPILER_ATTR_PACKED;

struct idt_entry {
	uint16_t base_low;
	uint16_t selector;
	uint8_t always_zero;
	uint8_t flags;
	uint16_t base_high;
} COMPILER_ATTR_PACKED;

struct idt_pointer {
	uint16_t size;
	uintptr_t base;
} COMPILER_ATTR_PACKED;

struct tss_entry {
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} COMPILER_ATTR_PACKED;

void gdt_set_entry(uint8_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void tss_set_kernel_stack(uintptr_t esp);

void gdt_install();
void idt_install();
void tss_install(uint16_t ss0, uintptr_t esp0);

extern void gdt_load(uintptr_t);
extern void idt_load(uintptr_t);
extern void tss_load();
typedef void (*idt_gate)(void);
