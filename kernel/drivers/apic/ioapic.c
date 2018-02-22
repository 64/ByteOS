#include "mm.h"
#include "libk.h"
#include "util.h"
#include "drivers/acpi.h"
#include "drivers/apic.h"

#define APIC_REG_ID 0
#define APIC_REG_VER 1
#define APIC_REG_REDTBL 16

#define APIC_IRQ_MASK 0x10000

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

static uint64_t ioapic_redtbl_read(virtaddr_t ioapic_base, uint8_t irq_line)
{
	uint32_t ioredtbl = (irq_line * 2) + APIC_REG_REDTBL;
	return ioapic_read(ioapic_base, ioredtbl) | ((uint64_t)ioapic_read(ioapic_base, ioredtbl + 1) << 32);
}

static void ioapic_redtbl_write(virtaddr_t ioapic_base, uint8_t irq_line, uint64_t value)
{
	uint32_t ioredtbl = (irq_line * 2) + APIC_REG_REDTBL;
	ioapic_write(ioapic_base, ioredtbl + 0, (uint32_t)value);
	ioapic_write(ioapic_base, ioredtbl + 1, (uint32_t)(value >> 32));
}

void ioapic_redirect(uint32_t gsi, uint8_t UNUSED(source), uint16_t flags, uint8_t target_apic)
{
	uint8_t target_apic_id = lapic_list[target_apic].id;
	struct madt_entry_ioapic *ioapic = gsi_to_ioapic(gsi);
	virtaddr_t ioapic_base = phys_to_virt(ioapic->phys_addr);
	
	uint64_t redirection = gsi + IRQ_APIC_BASE;
	if (flags & 2)
		redirection |= (1 << 13);
	if (flags & 8)
		redirection |= (1 << 15);
	redirection |= ((uint64_t)target_apic_id) << 56;

	ioapic_redtbl_write(ioapic_base, gsi - ioapic->gsi_base, redirection);
}

void ioapic_mask(uint32_t gsi)
{
	struct madt_entry_ioapic *ioapic = gsi_to_ioapic(gsi);
	virtaddr_t ioapic_base = phys_to_virt(ioapic->phys_addr);
	uint8_t irq_line = gsi - ioapic->gsi_base;
	uint64_t prev = ioapic_redtbl_read(ioapic_base, irq_line);
	ioapic_redtbl_write(ioapic_base, irq_line, prev | APIC_IRQ_MASK);
}

void ioapic_unmask(uint32_t gsi)
{
	struct madt_entry_ioapic *ioapic = gsi_to_ioapic(gsi);
	virtaddr_t ioapic_base = phys_to_virt(ioapic->phys_addr);
	uint8_t irq_line = gsi - ioapic->gsi_base;
	uint64_t prev = ioapic_redtbl_read(ioapic_base, irq_line);
	ioapic_redtbl_write(ioapic_base, irq_line, prev & (~APIC_IRQ_MASK));
}

uint8_t ioapic_isa_to_gsi(uint8_t isa)
{
	kassert_dbg(isa < 16);
	for (size_t i = 0; i < override_list_size; i++)
		if (override_list[i]->source == isa)
			return override_list[i]->gsi;
	return IRQ_APIC_BASE + isa;
}

uint8_t ioapic_gsi_to_isa(uint8_t gsi)
{
	for (size_t i = 0; i < override_list_size; i++)
		if (override_list[i]->gsi == gsi)
			return override_list[i]->source;
	return gsi;
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
