#include "libk.h"
#include "util.h"
#include "interrupts.h"
#include "percpu.h"
#include "mm.h"
#include "smp.h"

void ipi_send_fixed(cpuid_t id, uint8_t vec)
{
	uint8_t apic_id = percpu_table[id]->apic_id;
	lapic_send_ipi(apic_id, IPI_FIXED | vec);
}

void ipi_sched_hint(struct isr_ctx *UNUSED(regs))
{
	
}

void ipi_abort(struct isr_ctx *UNUSED(regs))
{
	abort_self();
}

// TODO: Add a mechanism for calling invlpg, not just reloading cr3
void ipi_tlb_shootdown(struct isr_ctx *UNUSED(regs))
{
	struct tlb_op *op = tlb_op_location;
	for (uintptr_t i = (uintptr_t)op->start; i < (uintptr_t)op->end; i += PAGE_SIZE) {
		tlb_flush_single((virtaddr_t)i);
	}
	atomic_dec_read32(&tlb_remaining_cpus);
}
