#include "interrupts.h"

void interrupts_init(void);

void interrupts_init(void)
{
	// Create and fill the IDT
	idt_init();

	// Mask PIC
	irq_init();

	// Execute 'lidt' instruction
	load_idt();
}
