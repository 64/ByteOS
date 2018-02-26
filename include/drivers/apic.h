#pragma once

#include <stddef.h>
#include "types.h"
#include "drivers/acpi.h"

#define MAX_LAPICS 16
#define MAX_IOAPICS 16
#define MAX_NMIS (2 * MAX_LAPICS)
#define MAX_OVERRIDES 48

#define IPI_INIT     0x4500
#define IPI_START_UP 0x4600
#define IPI_FIXED    0x4000

#define IPI_BROADCAST (0b11 << 18)

struct lapic_info {
	uint8_t id;
	uint8_t acpi_id;
	uint8_t present;
};

void apic_init(void);

void lapic_enable(void);
void lapic_eoi(uint8_t);
uint8_t lapic_id(void);
void lapic_send_ipi(uint8_t target, uint32_t flags);

void ioapic_init(void);
void ioapic_redirect(uint32_t gsi, uint8_t source, uint16_t flags, uint8_t target_apic);
void ioapic_mask(uint32_t gsi);
void ioapic_unmask(uint32_t gsi);
uint8_t ioapic_isa_to_gsi(uint8_t isa);
uint8_t ioapic_gsi_to_isa(uint8_t gsi);

extern struct lapic_info lapic_list[MAX_LAPICS];
extern struct madt_entry_ioapic *ioapic_list[MAX_IOAPICS];
extern struct madt_entry_override *override_list[MAX_OVERRIDES];
extern struct madt_entry_nmi *nmi_list[MAX_NMIS];
extern size_t lapic_list_size;
extern size_t ioapic_list_size;
extern size_t override_list_size;
extern size_t nmi_list_size;
extern virtaddr_t lapic_base;
