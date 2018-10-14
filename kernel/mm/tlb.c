#include "mm.h"
#include "percpu.h"
#include "proc.h"
#include "smp.h"
#include "libk.h"

#define MAX_TLB_OPS MAX_CORES

struct tlb_op *volatile tlb_op_location;
static spinlock_t shootdown_lock;
atomic32_t tlb_remaining_cpus;

void tlb_shootdown(struct mmu_info *mmu, virtaddr_t start, virtaddr_t end)
{
	struct tlb_op op = {
		.start = start,
		.end = end,
	};

	klog_verbose("tlb", "TLB shootdown range %p - %p\n", start, end);
	spin_lock(&shootdown_lock);
	spin_lock(&mmu->cpu_lock);

	tlb_op_location = &op;

	// Send the IPI
	// TODO: Find a more efficient method than sending IPIs one by one (logical destination mode?)
	for (size_t i = 0; i < smp_nr_cpus(); i++) {
		if (i != percpu_get(id) && cpuset_query_id(&mmu->cpus, i)) {
			atomic_inc_read32(&tlb_remaining_cpus);
			ipi_send_fixed(i, IRQ_IPI_TLB_SHOOTDOWN);
		}
	}

	// Wait for the other CPUs to finish
	while (atomic_read32(&tlb_remaining_cpus) != 0) {
		pause();
	}

	spin_unlock(&mmu->cpu_lock);
	spin_unlock(&shootdown_lock);
}

// TODO: Functions for lazy TLB invalidation

