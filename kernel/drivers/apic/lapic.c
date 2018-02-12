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
#define APIC_BASE_MSR 0x1B

#define APIC_REG_SPURIOUS 0xF0U
#define APIC_REG_EOI 0xB0
#define APIC_REG_LINT0 0x350U
#define APIC_REG_LINT1 0x360U

static bool has_lapic(void)
{
	uint32_t eax, ebx, ecx, edx = 0;
	__get_cpuid(1, &eax, &ebx, &ecx, &edx);
	return (edx & APIC_CPUID_BIT) != 0;
}

static physaddr_t find_lapic_base(struct acpi_madt *madt)
{
	(void)madt;
	return 0;
}

static inline void lapic_write(virtaddr_t apic_base, uint32_t reg_offset, uint32_t data)
{
	*(volatile uint32_t *)((uintptr_t)apic_base + reg_offset) = data;
}

static inline uint32_t lapic_read(virtaddr_t apic_base, uint32_t reg_offset)
{
	return *(volatile uint32_t *)((uintptr_t)apic_base + reg_offset);
}

void lapic_init(struct acpi_madt *madt)
{
	// TODO: Fallback to PIC
	if (!has_lapic())
		panic("no local APIC found"); 

	// Map the APIC base so we can access it
	virtaddr_t apic_base = phys_to_virt(find_lapic_base(madt));
	paging_map_page(kernel_p4, virt_to_phys(apic_base), apic_base, PAGE_DISABLE_CACHE | PAGE_WRITABLE);

	// Enable the LAPIC via the spurious interrupt register
	lapic_write(apic_base, APIC_REG_SPURIOUS, lapic_read(apic_base, APIC_REG_SPURIOUS) | 0x1FF);

	// Disable LINT0 and LINT1 (TODO: Set this to NMI according to MADT)
	lapic_write(apic_base, APIC_REG_LINT0, 1 << 16);
	lapic_write(apic_base, APIC_REG_LINT1, 1 << 16);

	lapic_write(apic_base, APIC_REG_EOI, 0); // Clear any pending interrupts
}

