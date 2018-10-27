#include "interrupts.h"

void interrupts_init(void);

void interrupts_init(void)
{
	// Create and fill the IDT
	idt_init();

	// Initialise ISR table and mask IRQs
	isr_init();

	// Write exception handlers into ISR table
	exceptions_init();

	// Execute 'lidt' instruction
	idt_load();
}
