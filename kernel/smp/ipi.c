#include "libk.h"
#include "util.h"
#include "interrupts.h"
#include "percpu.h"
#include "proc.h"
#include "mm.h"
#include "smp.h"

void ipi_send_fixed(cpuid_t id, uint8_t vec)
{
	uint8_t apic_id = percpu_table[id]->apic_id;
	lapic_send_ipi(apic_id, IPI_FIXED | vec);
}

// Begins running the scheduler
void ipi_sched_hint(struct isr_ctx *UNUSED(regs))
{
	lapic_eoi(IRQ_IPI_SCHED_HINT);
	sched_run_ap();
}

void ipi_abort(struct isr_ctx *UNUSED(regs))
{
	abort_self();
}

void ipi_tlb_shootdown(struct isr_ctx *UNUSED(regs))
{
	struct tlb_op *op = tlb_op_location;
	for (uintptr_t i = (uintptr_t)op->start; i < (uintptr_t)op->end; i += PAGE_SIZE) {
		tlb_flush_single((virtaddr_t)i);
	}
	atomic_dec_read32(&tlb_remaining_cpus);
	lapic_eoi(IRQ_IPI_TLB_SHOOTDOWN);
}
