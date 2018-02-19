#include "interrupts.h"
#include "drivers/apic.h"
#include "libk.h"
#include "asm.h"

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
}

static inline void irq_ack(void)
{
	lapic_send_eoi();
}

void irq_handler(struct stack_regs *regs)
{
	irq_ack();
	kprintf("Unhandled IRQ at %p\n", (void *)regs->rip);
}
