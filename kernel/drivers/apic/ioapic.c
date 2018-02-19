#include "mm.h"
#include "libk.h"
#include "drivers/acpi.h"
#include "drivers/apic.h"

#define APIC_REG_ID 0
#define APIC_REG_VER 1
#define APIC_REG_REDTBL 16

static inline uint32_t ioapic_read(volatile uint32_t *ioapic, uint8_t reg)
{
	ioapic[0] = (reg & 0xFF);
	return ioapic[4];
}

static inline void ioapic_write(volatile uint32_t *ioapic, uint8_t reg, uint32_t data)
{
	ioapic[0] = (reg & 0xFF);
	ioapic[4] = data;
}

static inline uint32_t get_max_redirs(size_t ioapic_id)
{
	virtaddr_t base = phys_to_virt(ioapic_list[ioapic_id]->phys_addr);
	return (ioapic_read(base, APIC_REG_VER) & 0xFF0000) >> 16;
}

static inline struct madt_entry_ioapic *gsi_to_ioapic(uint32_t gsi)
{
	for (size_t i = 0; i < ioapic_list_size; i++) {
		uint32_t max_redirs = get_max_redirs(i);
		if (ioapic_list[i]->gsi_base <= gsi && ioapic_list[i]->gsi_base + max_redirs > gsi)
			return ioapic_list[i];
	}
	panic("I/O APIC not found for GSI %u", gsi);
}

void ioapic_redirect(uint32_t gsi, uint8_t source, uint16_t flags, uint8_t target_apic)
{
	uint8_t target_apic_id = lapic_list[target_apic].id;
	struct madt_entry_ioapic *ioapic = gsi_to_ioapic(gsi);
	virtaddr_t ioapic_base = phys_to_virt(ioapic->phys_addr);
	
	uint64_t redirection = source + IRQ_APIC_BASE;
	if (flags & 2)
		redirection |= (1 << 13);
	if (flags & 8)
		redirection |= (1 << 15);
	redirection |= ((uint64_t)target_apic_id) << 56;

	uint32_t ioredtbl = (gsi - ioapic->gsi_base) * 2 + APIC_REG_REDTBL;
	ioapic_write(ioapic_base, ioredtbl + 0, (uint32_t)redirection);
	ioapic_write(ioapic_base, ioredtbl + 1, (uint32_t)(redirection >> 32));
}

void ioapic_init(void)
{
	// Initialised the (assumed) wirings for the legacy PIC IRQs
	// Send all IRQs to the BSP for simplicity
	for (uint8_t i = 0; i < 16; i++)
		ioapic_redirect(i, i, 0, 0);

	// Setup the actual overrides
	for (size_t i = 0; i < override_list_size; i++)
		ioapic_redirect(override_list[i]->gsi, override_list[i]->source, override_list[i]->flags, 0);
}
