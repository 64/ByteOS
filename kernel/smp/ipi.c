#include "libk.h"
#include "util.h"
#include "interrupts.h"
#include "smp.h"

void ipi_abort(struct isr_ctx *UNUSED(regs))
{
	abort_self();
}

void ipi_tlb_shootdown(struct isr_ctx *UNUSED(regs))
{
		
}
