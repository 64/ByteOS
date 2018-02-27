#include "libk.h"
#include "util.h"
#include "interrupts.h"
#include "smp.h"

void ipi_abort(struct isr_context *UNUSED(regs))
{
	abort_self();
}
