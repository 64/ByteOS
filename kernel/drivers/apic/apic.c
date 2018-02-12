#include <stdint.h>
#include <stdbool.h>
#include <cpuid.h>
#include "libk.h"
#include "system.h"
#include "drivers/apic.h"

#define APIC_CPUID_BIT (1 << 9)

static bool has_local_apic(void)
{
	uint32_t eax, ebx, ecx, edx = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	return (edx & APIC_CPUID_BIT) != 0;
}

void apic_init(void)
{
	if (!has_local_apic())
		panic("no local APIC found");
	kprintf("%p\n", (void *)msr_read(0xC0000082));
}

