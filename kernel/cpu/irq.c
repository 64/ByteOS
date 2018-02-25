#include "interrupts.h"
#include "drivers/apic.h"
#include "libk.h"
#include "asm.h"
#include "util.h"

static irq_handler_t irq_handlers[256];

void irq_init(void)
{
	// Remap the PIC to interrupts 0x20-0x2F
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x00);
	outb(0xA1, 0x00);

	// Disable the PIC by masking all interrupts
	outb(0xA1, 0xFF);
	outb(0x21, 0xFF);

	memset(irq_handlers, 0, sizeof irq_handlers);

	irq_disable();
}

void irq_register_handler(uint8_t vec, irq_handler_t irq)
{
	irq_handlers[vec] = irq;
}

void irq_mask(uint8_t vec)
{
	kassert_dbg(vec >= IRQ_APIC_BASE);
	ioapic_mask(vec - IRQ_APIC_BASE);
}

void irq_unmask(uint8_t vec)
{
	kassert_dbg(vec >= IRQ_APIC_BASE);
	ioapic_unmask(vec - IRQ_APIC_BASE);
}

void irq_eoi(uint8_t vec)
{
	lapic_eoi(vec);
}

void irq_handler(struct stack_regs *regs)
{
	uint8_t int_no = regs->info & 0xFF;
	irq_mask(int_no);
	irq_handler_t handler = irq_handlers[int_no];
	if (handler != NULL)
		handler(regs);
	else {
		kprintf("Unhandled IRQ %u at %p\n", int_no, (void *)regs->rip);
	}
	irq_eoi(int_no);
	irq_unmask(int_no);
}
