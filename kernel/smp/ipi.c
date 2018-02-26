#include "libk.h"
#include "util.h"
#include "interrupts.h"
#include "smp.h"

void ipi_abort(struct stack_regs *UNUSED(regs))
{
	abort_self();
}
