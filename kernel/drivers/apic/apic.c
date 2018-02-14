#include <stddef.h>
#include "libk.h"
#include "types.h"
#include "mm.h"
#include "drivers/apic.h"

struct lapic_info lapic_list[MAX_LAPICS];
struct ioapic_info ioapic_list[MAX_IOAPICS];
struct override_info override_list[MAX_OVERRIDES];
struct nmi_info nmi_list[MAX_NMIS];

struct madt_entry_header {
	uint8_t type;
	uint8_t length;
} __attribute__((packed));

#define MADT_LAPIC 0
struct madt_entry_lapic {
	struct madt_entry_header header;
	uint8_t acpi_id;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__((packed));

#define MADT_IOAPIC 1
struct madt_entry_ioapic {
	struct madt_entry_header header;
	uint8_t apic_id;
	uint8_t __zero;
	uint32_t phys_addr;
	uint32_t gsi_base;
} __attribute__((packed));

#define MADT_OVERRIDE 2
struct madt_entry_override {
	struct madt_entry_header header;
	uint8_t bus; // Constant, set to 0
	uint8_t source;
	uint32_t gsi;
	uint16_t flags;
} __attribute__((packed));

#define MADT_NMI 4
struct madt_entry_nmi {
	struct madt_entry_header header;
	uint8_t acpi_id;
	uint16_t flags;
	uint8_t lint_num;
} __attribute__((packed));

#define MADT_LAPIC_ADDR 5
struct madt_entry_lapic_addr {
	struct madt_entry_header header;
	uint16_t __zero;
	uint64_t lapic_addr;
} __attribute__((packed));

static void add_lapic(size_t index, struct madt_entry_lapic *entry)
{
	if (index >= MAX_LAPICS)
		return;
	lapic_list[index].present = (entry->flags != 0);
	lapic_list[index].id = entry->apic_id;
	kprintf("acpi: Detected local APIC, id %d\n", entry->apic_id);
}

static void add_ioapic(size_t index, struct madt_entry_ioapic *entry)
{
	if (index >= MAX_IOAPICS)
		return;
	ioapic_list[index].present = 1;
	ioapic_list[index].id = entry->apic_id;
	kprintf("acpi: Detected I/O APIC, id %d\n", entry->apic_id);
}

static void add_override(size_t index, struct madt_entry_override *entry)
{
	if (index >= MAX_OVERRIDES)
		return;
	override_list[index].source = entry->source;
	override_list[index].gsi = entry->gsi;
	kprintf("acpi: GSI %d override\n", entry->gsi);
}

static void add_nmi(size_t index, struct madt_entry_nmi *entry)
{
	if (index >= MAX_NMIS)
		return;
	nmi_list[index].acpi_id = entry->acpi_id;
	nmi_list[index].lint_num = entry->lint_num;
}

static void parse_madt(struct acpi_madt *madt)
{
	// Default LAPIC address (might be overridden by entry type 5)
	lapic_base = phys_to_virt(madt->lapic_address);

	// Parse all the other entries
	struct madt_entry_header *hd = (struct madt_entry_header *)&madt->entries;
	struct madt_entry_header *end = (struct madt_entry_header *)((uintptr_t)madt + madt->header.length);

	size_t lapic_index = 0, ioapic_index = 0, override_index = 0, nmi_index = 0;
	while (hd < end) {
		switch (hd->type) {
			case MADT_LAPIC:
				add_lapic(lapic_index++, (struct madt_entry_lapic *)hd);
				break;
			case MADT_IOAPIC:
				add_ioapic(ioapic_index++, (struct madt_entry_ioapic *)hd);
				break;
			case MADT_OVERRIDE:
				add_override(override_index++, (struct madt_entry_override *)hd);
				break;
			case MADT_NMI:
				add_nmi(nmi_index++, (struct madt_entry_nmi *)hd);
				break;
			case MADT_LAPIC_ADDR:
				lapic_base = phys_to_virt(((struct madt_entry_lapic_addr *)hd)->lapic_addr);
				break;
			default:
				kprintf("acpi[warn]: Unrecognised entry type %d in MADT\n", hd->type);
				break;
		}
		hd = (struct madt_entry_header *)((uintptr_t)hd + hd->length);
	}

	kprintf("acpi: Local APIC base at %p\n", lapic_base);

	// Map the APIC base so we can access it
	paging_map_page(kernel_p4, virt_to_phys(lapic_base), lapic_base, PAGE_DISABLE_CACHE | PAGE_WRITABLE);
}

void apic_init(void)
{
	struct acpi_madt *madt = acpi_find_table("APIC");
	if (madt == NULL)
		panic("no MADT found in ACPI tables");

	parse_madt(madt);
}
