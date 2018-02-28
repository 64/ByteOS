#include "asm.h"
#include "mm.h"
#include "smp.h"
#include "percpu.h"

void percpu_init(void)
{
	struct percpu *cpu = kmalloc(sizeof * cpu, KM_NONE);
	msr_write(0xC0000101, (uint64_t)cpu);

	cpu->id = smp_cpu_id();
	cpu->task = NULL;
	cpu->rsp_scratch = NULL;
	cpu->need_reschedule = false;
}
