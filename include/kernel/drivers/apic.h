#pragma once

#include "drivers/acpi.h"

void lapic_init(struct acpi_madt *);
void ioapic_init(struct acpi_madt *);
void apic_init(void);
