#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "interrupts.h"
#include "limits.h"

typedef uint32_t cpuset_t;
typedef uint8_t cpuid_t;

cpuid_t smp_cpu_id(void);
void smp_init(void);
void smp_ap_kmain(void);

void ipi_abort(struct isr_ctx *regs);
void ipi_tlb_shootdown(struct isr_ctx *regs);

void cpuset_init(cpuset_t *cpus);
void cpuset_copy(cpuset_t *dest, cpuset_t *src);
bool cpuset_query_id(cpuset_t *cpus, cpuid_t id);
void cpuset_set_id(cpuset_t *cpus, cpuid_t id, bool val);

#ifdef VERBOSE
void cpuset_dump(cpuset_t *cpus);
#endif

extern volatile unsigned int smp_nr_cpus_ready;

static inline unsigned int smp_nr_cpus(void)
{
	return __atomic_load_n(&smp_nr_cpus_ready, __ATOMIC_RELAXED);
}
