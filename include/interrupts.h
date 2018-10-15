#pragma once

#include <stdint.h>
#include "asm.h"
#include "drivers/apic.h"

#define IST_NONE 0
#define IST_NMI 1
#define IST_DOUBLE_FAULT 2

#define IRQ_APIC_SPURIOUS 0xFF
#define IRQ_APIC_BASE 0x30
#define IRQ_LINT_BASE (IRQ_APIC_SPURIOUS - 3) // One for each LINT pin, one for timer
#define IRQ_LINT_TIMER (IRQ_LINT_BASE + 2)
#define IRQ_NMI (IRQ_LINT_BASE - 1)

#define IRQ_IPI_TOP (IRQ_NMI - 1)
#define IRQ_IPI_ABORT (IRQ_IPI_TOP - 0)
#define IRQ_IPI_TLB_SHOOTDOWN (IRQ_IPI_TOP - 1)
#define IRQ_IPI_SCHED_HINT (IRQ_IPI_TOP - 2)

#define ISA_TO_INTERRUPT(x) (ioapic_isa_to_gsi(x) + IRQ_APIC_BASE)

struct isr_ctx {
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
	// Contains error code and interrupt number for exceptions
	// Contains syscall number for syscalls
	// Contains just the interrupt number otherwise
	uint64_t info;
	// Interrupt stack frame
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

typedef void (*irq_handler_t)(struct isr_ctx *);

struct idt_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist_index;
	uint8_t type_attr;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t __zero;
};

extern struct idt_entry idt64[256];

void exception_handler(struct isr_ctx *);

void idt_init(void);
void idt_set_isr(uint8_t index, virtaddr_t entry, uint8_t ist, uint8_t type_attr);

void load_idt(void);

void irq_init(void);
void irq_handler(struct isr_ctx *);
void irq_eoi(uint8_t);
void irq_mask(uint8_t);
void irq_unmask(uint8_t);
void irq_register_handler(uint8_t vec, irq_handler_t irq);
