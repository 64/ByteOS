#include <stdint.h>
#include <stdbool.h>
#include <cpuid.h>
#include "libk.h"
#include "types.h"
#include "cpu.h"
#include "mm.h"
#include "util.h"
#include "drivers/acpi.h"
#include "drivers/apic.h"

#define APIC_CPUID_BIT (1 << 9)

#define APIC_REG_SPURIOUS 0xF0U
#define APIC_REG_EOI 0xB0
#define APIC_REG_LINT0 0x350U
#define APIC_REG_LINT1 0x360U

virtaddr_t lapic_base; // Shared by all CPUs

static bool has_lapic(void)
{
	uint32_t eax, ebx, ecx, edx = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	return (edx & APIC_CPUID_BIT) != 0;
}

static inline void lapic_write(uint32_t reg_offset, uint32_t data)
{
	*(volatile uint32_t *)((uintptr_t)lapic_base + reg_offset) = data;
}

static inline uint32_t lapic_read(uint32_t reg_offset)
{
	return *(volatile uint32_t *)((uintptr_t)lapic_base + reg_offset);
}

void lapic_enable(struct lapic_info *lapic)
{
	// TODO: Fallback to PIC
	if (!has_lapic())
		panic("no local APIC found for current CPU"); 

	// Enable the LAPIC via the spurious interrupt register
	lapic_write(APIC_REG_SPURIOUS, lapic_read(APIC_REG_SPURIOUS) | 0x1FF);

	// TODO: Handle LINT0 and LINT1
	(void)lapic;

	lapic_write(APIC_REG_EOI, 0); // Clear any pending interrupts
}

