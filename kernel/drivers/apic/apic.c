#include <stddef.h>
#include "libk.h"
#include "types.h"
#include "util.h"
#include "interrupts.h"
#include "mm.h"
#include "drivers/apic.h"

// Thanks to https://nemez.net/osdev/index.html for the information and sample code

struct lapic_info lapic_list[MAX_LAPICS];
struct madt_entry_ioapic *ioapic_list[MAX_IOAPICS];
struct madt_entry_override *override_list[MAX_OVERRIDES];
struct madt_entry_nmi *nmi_list[MAX_NMIS];
size_t lapic_list_size;
size_t ioapic_list_size;
size_t override_list_size;
size_t nmi_list_size;

static void add_lapic(struct madt_entry_lapic *entry)
{
	if (lapic_list_size >= MAX_LAPICS)
		return;
	lapic_list[lapic_list_size].present = (entry->flags != 0);
	lapic_list[lapic_list_size].id = entry->apic_id;
	lapic_list[lapic_list_size].acpi_id = entry->acpi_id;
	lapic_list_size++;
	klog("apic", "Detected local APIC, id %d\n", entry->apic_id);
}

static void add_ioapic(struct madt_entry_ioapic *entry)
{
	if (ioapic_list_size >= MAX_IOAPICS)
		return;
	// Map the I/O APIC base so we can access it
	paging_map_page(kernel_p4, entry->phys_addr, phys_to_virt(entry->phys_addr), PAGING_ALLOC_MMAP | PAGE_DISABLE_CACHE | PAGE_WRITABLE);
	ioapic_list[ioapic_list_size++] = entry;
	klog("apic", "Detected I/O APIC, id %d\n", entry->apic_id);
}

static void add_override(struct madt_entry_override *entry)
{
	if (override_list_size >= MAX_OVERRIDES)
		return;
	override_list[override_list_size++] = entry;
	klog("apic", "GSI %d overrides IRQ %u, flags %x\n", entry->gsi, entry->source, entry->flags);
}

static void add_nmi(struct madt_entry_nmi *entry)
{
	if (nmi_list_size >= MAX_NMIS)
		return;
	nmi_list[nmi_list_size++] = entry;
	if (entry->acpi_id == 0xFF)
		klog("apic", "NMI for all CPUs, LINT%d\n", entry->lint_num);
	else
		klog("apic", "NMI for CPU %d, LINT%d\n", entry->acpi_id, entry->lint_num);

}

static void parse_madt(struct acpi_madt *madt)
{
	// Default LAPIC address (might be overridden by entry type 5)
	virtaddr_t tmp_lapic_base = phys_to_virt(madt->lapic_address);

	// Parse all the other entries
	struct madt_entry_header *hd = (struct madt_entry_header *)&madt->entries;
	struct madt_entry_header *end = (struct madt_entry_header *)((uintptr_t)madt + madt->header.length);

	while (hd < end) {
		switch (hd->type) {
			case MADT_LAPIC:
				add_lapic((struct madt_entry_lapic *)hd);
				break;
			case MADT_IOAPIC:
				add_ioapic((struct madt_entry_ioapic *)hd);
				break;
			case MADT_OVERRIDE:
				add_override((struct madt_entry_override *)hd);
				break;
			case MADT_NMI:
				add_nmi((struct madt_entry_nmi *)hd);
				break;
			case MADT_LAPIC_ADDR:
				tmp_lapic_base = phys_to_virt(((struct madt_entry_lapic_addr *)hd)->lapic_addr);
				break;
			default:
				klog_warn("apic", "Unrecognised entry type %d in MADT\n", hd->type);
				break;
		}
		hd = (struct madt_entry_header *)((uintptr_t)hd + hd->length);
	}

	lapic_base = tmp_lapic_base;
	klog("apic", "Local APIC base at %p\n", lapic_base);

	// Map the APIC base so we can access it
	paging_map_page(kernel_p4, virt_to_phys(lapic_base), lapic_base, PAGING_ALLOC_MMAP | PAGE_DISABLE_CACHE | PAGE_WRITABLE);
}

void apic_init(void)
{
	struct acpi_madt *madt = acpi_find_table("APIC");
	if (madt == NULL)
		panic("no MADT found in ACPI tables");

	parse_madt(madt);
}
