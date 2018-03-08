#include "libk.h"
#include "util.h"
#include "interrupts.h"
#include "smp.h"

void ipi_abort(struct isr_ctx *UNUSED(regs))
{
	abort_self();
}

// TODO: Add a mechanism for calling invlpg, not just reloading cr3
void ipi_tlb_shootdown(struct isr_ctx *UNUSED(regs))
{
	reload_cr3();			
}
