#include <stdint.h>
#include <stdbool.h>
#include <cpuid.h>
#include "libk.h"
#include "cpu.h"
#include "drivers/apic.h"

#define APIC_CPUID_BIT (1 << 9)

static bool has_lapic(void)
{
	uint32_t eax, ebx, ecx, edx = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	return (edx & APIC_CPUID_BIT) != 0;
}

void lapic_init(void)
{
	if (!has_lapic())
		panic("no local APIC found"); // TODO: Fallback to PIC
	kprintf("%p\n", (void *)msr_read(0xC0000082));
}

