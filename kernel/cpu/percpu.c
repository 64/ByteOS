#include "asm.h"
#include "mm.h"
#include "smp.h"
#include "percpu.h"

extern uint64_t tss64[];

void percpu_init(void)
{
	struct percpu *cpu = kmalloc(sizeof * cpu, KM_NONE);
	percpu_set_addr(cpu);

	cpu->id = smp_cpu_id();
	cpu->current = NULL;
	cpu->rsp_scratch = NULL;
	cpu->preempt_count = 0;

	if (cpu->id == 0)
		cpu->tss = tss64;
	else
		cpu->tss = 0x0; // TODO
}

void percpu_set_addr(struct percpu *p)
{
	// MSR_GS_BASE
	msr_write(0xC0000101, (uint64_t)p);
}
