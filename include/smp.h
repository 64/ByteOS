#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t cpuset_t;
typedef uint8_t cpuid_t;

#include "interrupts.h"
#include "atomic.h"
#include "limits.h"
#include "percpu.h"
cpuid_t smp_cpu_id_full(void);
void smp_init(void);
void smp_ap_kmain(void);


void ipi_send_fixed(cpuid_t, uint8_t vec);
void ipi_sched_hint(struct isr_ctx *regs);
void ipi_abort(struct isr_ctx *regs);
void ipi_tlb_shootdown(struct isr_ctx *regs);

void cpuset_init(cpuset_t *cpus);
void cpuset_clear(cpuset_t *cpus);
void cpuset_pin(cpuset_t *cpus);
void cpuset_unpin(cpuset_t *cpus);
bool cpuset_is_pinned(cpuset_t *cpus);
void cpuset_copy(cpuset_t *dest, cpuset_t *src);
bool cpuset_query_id(cpuset_t *cpus, cpuid_t id);
void cpuset_set_id(cpuset_t *cpus, cpuid_t id, bool val);

#ifdef VERBOSE
void cpuset_dump(cpuset_t *cpus);
#endif

extern atomic32_t smp_nr_cpus_ready;

static inline uint32_t smp_nr_cpus(void)
{
	return atomic_read32(&smp_nr_cpus_ready);
}

static inline cpuid_t smp_cpu_id(void)
{
	return percpu_get(id);
}
