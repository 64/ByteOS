#include <stdint.h>
#include <stdbool.h>
#include <cpuid.h>
#include "libk.h"
#include "types.h"
#include "mm.h"
#include "util.h"
#include "drivers/acpi.h"
#include "drivers/apic.h"

#define APIC_CPUID_BIT (1 << 9)

#define APIC_REG_ID 0x20
#define APIC_REG_EOI 0xB0
#define APIC_REG_SPURIOUS 0xF0U
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

static inline struct lapic_info *find_lapic(uint8_t id)
{
	for (size_t i = 0; i < lapic_list_size; i++)
		if (lapic_list[i].id == id)
			return &lapic_list[i];
	return NULL;
}

static inline void lapic_set_nmi(uint8_t vec, struct madt_entry_nmi *nmi_info)
{
	kassert_dbg(vec >= IRQ_NMI_BASE && vec < IRQ_APIC_SPURIOUS);
	uint32_t nmi = 800 | vec;
	if (nmi_info->flags & 2)
		nmi |= (1 << 13);
	if (nmi_info->flags & 8)
		nmi |= (1 << 15);
	if (nmi_info->lint_num == 0)
		lapic_write(APIC_REG_LINT0, nmi);
	else if (nmi_info->lint_num == 1)
		lapic_write(APIC_REG_LINT1, nmi);
}

uint8_t lapic_id(void)
{
	return lapic_read(APIC_REG_ID) >> 24;
}

void lapic_eoi(uint8_t vec)
{
	// TODO: Check the corresponding bit in the LAPIC's ISR
	if (vec != IRQ_NMI_BASE && vec != IRQ_NMI_BASE + 1)
		lapic_write(APIC_REG_EOI, 0);
}

void lapic_enable(void)
{
	// TODO: Fallback to PIC
	if (!has_lapic())
		panic("No local APIC found for current CPU"); 

	uint8_t id = lapic_id();
	struct lapic_info *lapic = find_lapic(id);

	for (size_t i = 0; i < nmi_list_size; i++)
		if (nmi_list[i]->acpi_id == lapic->acpi_id || nmi_list[i]->acpi_id == 0xFF)
			lapic_set_nmi(IRQ_NMI_BASE + nmi_list[i]->lint_num, nmi_list[i]);

	// Enable the LAPIC via the spurious interrupt register
	lapic_write(APIC_REG_SPURIOUS, lapic_read(APIC_REG_SPURIOUS) | (1 << 8) | IRQ_APIC_SPURIOUS);
	
	// Clear any pending interrupts
	lapic_write(APIC_REG_EOI, 0);
	klog("apic", "CPU %u online\n", id);
}

