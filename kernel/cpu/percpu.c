#include "asm.h"
#include "mm.h"
#include "smp.h"
#include "percpu.h"

extern uint64_t tss64[];

struct percpu *percpu_table[MAX_CORES] = { 0 };

static struct percpu bsp_percpu = {
	.tss = tss64
};

void percpu_init_ap(void)
{
	// Initialise a temp struct just for the kmalloc call (since it will lock)
	struct percpu tmp = { 0 };
	percpu_set_addr(&tmp);

	struct percpu *cpu = kmalloc(sizeof * cpu, KM_NONE);
	cpu->id = smp_cpu_id();
	cpu->current = NULL;
	cpu->rsp_scratch = NULL;
	cpu->preempt_count = 0;
	cpu->tss = 0x0; // TODO
	percpu_table[cpu->id] = cpu;
	percpu_set_addr(cpu);
}

void percpu_init_bsp(void)
{
	percpu_table[0] = &bsp_percpu;
	percpu_set_addr(&bsp_percpu);
}

void percpu_set_addr(struct percpu *p)
{
	// MSR_GS_BASE
	msr_write(0xC0000101, (uint64_t)p);
}
