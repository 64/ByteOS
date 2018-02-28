#include <stddef.h>

#include "libk.h"
#include "mm.h"
#include "drivers/acpi.h"

#define RSDP_SEARCH_START 0xE0000
#define RSDP_SEARCH_END   0xFFFFF

struct rsdp {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t ext_checksum;
	uint8_t reserved[3];
} __attribute__((packed));

struct rsdt {
	struct acpi_header header;
	uint32_t tables[];
} __attribute__((packed));

struct xsdt {
	struct acpi_header header;
	physaddr_t tables[];
} __attribute__((packed));

struct rsdp *acpi_rsdp;

static uint8_t calc_checksum(uint8_t *bytes, size_t len)
{
	uint8_t sum = 0;
	for (size_t i = 0; i < len; i++)
		sum += bytes[i];
	return sum;
}

static struct rsdp *find_rsdp(void)
{
	for (physaddr_t phys = RSDP_SEARCH_START; phys < RSDP_SEARCH_END; phys += 16) {
		virtaddr_t virt = phys_to_virt(phys);
		if (memcmp("RSD PTR ", virt, 8) == 0) {
			struct rsdp *rsdp = virt;
			if (rsdp->revision == 0) { // Version 1
				if (calc_checksum(virt, sizeof(struct rsdp) - 16) != 0)
					continue;
			} else if (rsdp->revision == 2) { // Version 2+
				if (calc_checksum(virt, sizeof(struct rsdp)) != 0)
					continue;
			} else
				continue;
			return rsdp;
		}
	}
	return NULL;
}

// Every table we access needs to be called here first
static void map_table(struct acpi_header *hd)
{
	// Map the first two pages for safety, then map the rest of the table
	// The header could cross a page boundary and generate a page fault.
	paging_map_page(kernel_p4, virt_to_phys(hd), hd, PAGING_NONE);
	paging_map_page(kernel_p4, virt_to_phys(hd) + sizeof(struct acpi_header),
			(virtaddr_t)((uintptr_t)hd + sizeof(struct acpi_header)), PAGING_NONE);

	// Safe to dereference header now

	// Map the rest of the table
	for (size_t i = sizeof(struct acpi_header) + PAGE_SIZE; i < hd->length; i += PAGE_SIZE) {
		virtaddr_t virt = (virtaddr_t)((uintptr_t)hd + i);
		physaddr_t phys = virt_to_phys(virt);
		paging_map_page(kernel_p4, phys, virt, PAGING_NONE);
	}

	// Safe to access the whole table now
}

static inline bool table_checksum(struct acpi_header *hd)
{
	return calc_checksum((uint8_t *)hd, hd->length) == 0;
}

static inline void print_table(struct acpi_header *hd)
{
	klog("acpi", "%c%c%c%c at %p, OEM %c%c%c%c%c%c\n",
	     hd->signature[0], hd->signature[1], hd->signature[2], hd->signature[3], (virtaddr_t)virt_to_phys(hd),
	     hd->oem_id[0], hd->oem_id[1], hd->oem_id[2], hd->oem_id[3], hd->oem_id[4], hd->oem_id[5]);
#ifdef VERBOSE
	klog("acpi", "  Length: %u\n", hd->length);
	klog("acpi", "  Revision: %u\n", hd->revision);
	klog("acpi", "  OEM Table ID: %c%c%c%c%c%c%c%c\n", hd->oem_table_id[0], hd->oem_table_id[1], hd->oem_table_id[2],
	     hd->oem_table_id[3], hd->oem_table_id[4], hd->oem_table_id[5], hd->oem_table_id[6], hd->oem_table_id[7]);
	klog("acpi", "  OEM Revision: %u\n", hd->oem_revision);
	klog("acpi", "  Creator ID: %c%c%c%c\n", hd->creator_id[0], hd->creator_id[1], hd->creator_id[2], hd->creator_id[3]);
	klog("acpi", "  Creator Revision: %u\n", hd->creator_revision);
#endif
}

static void acpi_dump_tables(void)
{
	if (acpi_rsdp->revision == 0) {
		// Use ACPI v1 RSDT
		struct rsdt *rsdt = phys_to_virt(acpi_rsdp->rsdt_address);
		map_table(&rsdt->header);
		if (!table_checksum(&rsdt->header))
			panic("acpi: RSDT checksum failed");
		size_t num_entries = (rsdt->header.length - sizeof(struct acpi_header)) / sizeof(uint32_t);
		for (size_t i = 0; i < num_entries; i++) {
			struct acpi_header *hd = phys_to_virt(rsdt->tables[i]);
			map_table(hd);
			print_table(hd);
		}
	} else if (acpi_rsdp->revision == 2) {
		// Use ACPI v2+ XSDT
		struct xsdt *xsdt = phys_to_virt(acpi_rsdp->xsdt_address);
		map_table(&xsdt->header);
		if (!table_checksum(&xsdt->header))
			panic("acpi: XSDT checksum failed");
		size_t num_entries = (xsdt->header.length - sizeof(struct acpi_header)) / sizeof(physaddr_t);
		for (size_t i = 0; i < num_entries; i++) {
			struct acpi_header *hd = phys_to_virt(xsdt->tables[i]);
			map_table(hd);
			print_table(hd);
		}
	}
}

virtaddr_t acpi_find_table(char *signature)
{
	// No need for map and checksum, we already did that above
	if (acpi_rsdp->revision == 0) {
		// Use ACPI v1 RSDT
		struct rsdt *rsdt = phys_to_virt(acpi_rsdp->rsdt_address);
		size_t num_entries = (rsdt->header.length - sizeof(struct acpi_header)) / sizeof(uint32_t);
		for (size_t i = 0; i < num_entries; i++) {
			struct acpi_header *hd = phys_to_virt(rsdt->tables[i]);
			if (memcmp(hd->signature, signature, 4) == 0 && table_checksum(hd))
				return hd;
		}
		return NULL;
	} else if (acpi_rsdp->revision == 2) {
		// Use ACPI v2+ XSDT
		struct xsdt *xsdt = phys_to_virt(acpi_rsdp->xsdt_address);
		size_t num_entries = (xsdt->header.length - sizeof(struct acpi_header)) / sizeof(physaddr_t);
		for (size_t i = 0; i < num_entries; i++) {
			struct acpi_header *hd = phys_to_virt(xsdt->tables[i]);
			if (memcmp(hd->signature, signature, 4) == 0 && table_checksum(hd))
				return hd;
		}
		return NULL;
	} else
		return NULL;
}

void acpi_init(void)
{
	acpi_rsdp = find_rsdp();
	if (acpi_rsdp == NULL)
		panic("ACPI: no RSDP found");
	acpi_dump_tables();
}
