#include "asm.h"
#include "mm.h"
#include "smp.h"
#include "percpu.h"

extern uint64_t tss64[];

void percpu_init(void)
{
	struct percpu *cpu = kmalloc(sizeof * cpu, KM_NONE);
	msr_write(0xC0000101, (uint64_t)cpu);

	cpu->id = smp_cpu_id();
	cpu->current = NULL;
	cpu->rsp_scratch = NULL;

	if (cpu->id == 0)
		cpu->tss = tss64;
	else
		cpu->tss = 0x0;
}
