#pragma once

#include <stddef.h>
#include "types.h"
#include "drivers/acpi.h"

#define MAX_LAPICS 16
#define MAX_IOAPICS 16
#define MAX_NMIS (2 * MAX_LAPICS)
#define MAX_OVERRIDES 48

#define IRQ_APIC_SPURIOUS 0xFF
#define IRQ_APIC_BASE 0x30
#define IRQ_NMI_BASE (IRQ_APIC_SPURIOUS - MAX_NMIS)

struct lapic_info {
	uint8_t id;
	uint8_t acpi_id;
	uint8_t present;
};

void apic_init(void);
void lapic_enable(void);
void lapic_send_eoi(void);
void ioapic_redirect(struct madt_entry_override *override, uint8_t target_apic);

extern struct lapic_info lapic_list[MAX_LAPICS];
extern struct madt_entry_ioapic *ioapic_list[MAX_IOAPICS];
extern struct madt_entry_override *override_list[MAX_OVERRIDES];
extern struct madt_entry_nmi *nmi_list[MAX_NMIS];
extern size_t lapic_list_size;
extern size_t ioapic_list_size;
extern size_t override_list_size;
extern size_t nmi_list_size;
extern virtaddr_t lapic_base;
