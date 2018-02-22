#include "smp.h"
#include "libk.h"
#include "drivers/apic.h"
#include "spin.h"

// This is slow, but we store the value in the per-CPU data structure, so use that instead
uint8_t smp_cpu_id(void)
{
	uint8_t lid = lapic_id();
	for (size_t i = 0; i < lapic_list_size; i++)
		if (lapic_list[i].id == lid)
			return i;
	panic("CPU not found in lapic_list");
}

// Boots all the cores
void smp_init(void)
{
	klog("smp", "Starting SMP boot sequence\n");
}
