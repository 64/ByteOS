#include "libk.h"
#include "util.h"
#include "interrupts.h"
#include "drivers/pit.h"
#include "drivers/apic.h"

// This is very dangerous to use after we have booted other cores
static volatile uint64_t ticks;

static void pit_irq(struct stack_regs *UNUSED(regs));

void pit_init(void)
{
	ticks = 0;
	irq_register_handler(ISA_TO_INTERRUPT(0), pit_irq);
	irq_unmask(ISA_TO_INTERRUPT(0)); // Unmask ourselves
}

static void pit_irq(struct stack_regs *UNUSED(regs))
{
	ticks++;
}

// TODO: Use milliseconds
void pit_sleep_ms(uint64_t ms)
{
	uint64_t target = ticks + ms;
	while (ticks < target)
		;
}
