#include "libk.h"
#include "interrupts.h"
#include "drivers/apic.h"

__attribute__((noreturn)) void abort_self(void)
{
	while (1) {
		asm volatile (
			"cli\n"
			"hlt\n"
			:
			:
			:
		);
	}
	__builtin_unreachable();
}

__attribute__((noreturn)) void abort(void)
{
	lapic_send_ipi(0, IPI_BROADCAST | IPI_FIXED | IRQ_IPI_ABORT);
	abort_self();
}

