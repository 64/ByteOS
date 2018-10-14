#include "asm.h"
#include "mm.h"
#include "smp.h"
#include "percpu.h"
#include "libk.h"

extern uint64_t tss64[];

struct percpu *percpu_table[MAX_CORES] = { 0 };

static struct percpu bsp_percpu = {
	.tss = tss64
};

static virtaddr_t load_gdt_tss(void)
{
	extern virtaddr_t gdt64;

	// Load GDT
	const size_t GDT_SIZE = 0x40;
	virtaddr_t gdt = kmalloc(GDT_SIZE, KM_NONE);
	memcpy(gdt, &gdt64, GDT_SIZE);

	const size_t TSS_SIZE = 0x68;
	virtaddr_t tss = kmalloc(TSS_SIZE, KM_NONE);
	memset(tss, 0, TSS_SIZE);
	// TODO: Create IST entries for NMI, double fault	

	// Set TSS descriptor in GDT
	uint8_t tmp[8]; memcpy(tmp, &tss, sizeof(virtaddr_t));
	uint8_t *gdt_tss_desc = (uint8_t *)gdt + 0x30;
	gdt_tss_desc[2] = tmp[0];
	gdt_tss_desc[3] = tmp[1];
	gdt_tss_desc[4] = tmp[2];
	gdt_tss_desc[5] = 0x89; // Type
	gdt_tss_desc[7] = tmp[3];
	gdt_tss_desc[8] = tmp[4];
	gdt_tss_desc[9] = tmp[5];
	gdt_tss_desc[10] = tmp[6];
	gdt_tss_desc[11] = tmp[7];

	// Load GDT and TSS
	extern void flush_gdt_tss(virtaddr_t, uint16_t, virtaddr_t);
	flush_gdt_tss(gdt, GDT_SIZE - 1, tss);	
	
	return tss;
}

void percpu_init_ap(void)
{
	// Initialise a temp struct just for the kmalloc call (since it will lock)
	struct percpu tmp = { 0 };
	percpu_set_addr(&tmp);

	struct percpu *cpu = kmalloc(sizeof *cpu, KM_NONE);
	cpu->id = smp_cpu_id();
	cpu->current_task = NULL;
	cpu->rsp_scratch = NULL;
	cpu->preempt_count = 0;
	cpu->tss = load_gdt_tss();
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
