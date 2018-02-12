#include <stddef.h>
#include "libk.h"
#include "drivers/apic.h"

void apic_init(void)
{
	struct acpi_madt *madt = acpi_find_table("APIC");
	if (madt == NULL)
		panic("no MADT found in ACPI tables");

	lapic_init(madt);
	ioapic_init(madt);
}
