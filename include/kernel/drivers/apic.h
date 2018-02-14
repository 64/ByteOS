#pragma once

#include "types.h"
#include "drivers/acpi.h"

#define MAX_LAPICS 16
#define MAX_IOAPICS 16
#define MAX_NMIS (2 * MAX_LAPICS)
#define MAX_OVERRIDES 48

struct lapic_info {
	uint8_t id;
	uint8_t present;
};

struct ioapic_info {
	uint8_t id;
	uint8_t present;
};

struct override_info {
	uint8_t source;
	uint32_t gsi;
};

struct nmi_info {
	uint8_t acpi_id;
	uint8_t lint_num;
};

extern struct lapic_info lapic_list[MAX_LAPICS];
extern struct ioapic_info ioapic_list[MAX_IOAPICS];
extern struct override_info override_list[MAX_OVERRIDES];
extern struct nmi_info nmi_list[MAX_NMIS];

void apic_init(void);
void lapic_enable(struct lapic_info *lapic);
void ioapic_enable(virtaddr_t ioapic_base, struct ioapic_info *ioapic);

extern virtaddr_t lapic_base;
