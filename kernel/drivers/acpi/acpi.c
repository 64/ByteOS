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
	paging_map_page(kernel_p4, virt_to_phys(hd), hd, PAGING_NONE);
	paging_map_page(kernel_p4, virt_to_phys(hd) + sizeof(struct acpi_header),
			(virtaddr_t)((uintptr_t)hd + sizeof(struct acpi_header)), PAGING_NONE);

	// Safe to dereference header now

	// TODO: Map the rest of the table
	
	// Safe to access the whole table now
}

static inline bool table_checksum(struct acpi_header *hd)
{
	return calc_checksum((uint8_t *)hd, hd->length) == 0;
}

// TODO: Only map and check the tables once, not once per call
virtaddr_t acpi_find_table(char *signature)
{
	if (acpi_rsdp->revision == 0) {
		// Use ACPI v1 RSDT
		struct rsdt *rsdt = phys_to_virt(acpi_rsdp->rsdt_address);
		map_table(&rsdt->header);
		if (!table_checksum(&rsdt->header))
			return NULL;
		size_t num_entries = (rsdt->header.length - sizeof(struct acpi_header)) / sizeof(uint32_t);
		for (size_t i = 0; i < num_entries; i++) {
			struct acpi_header *hd = phys_to_virt(rsdt->tables[i]);
			map_table(hd);
			if (memcmp(hd->signature, signature, 4) == 0 && table_checksum(hd))
				return hd;
		}
		return NULL;
	} else if (acpi_rsdp->revision == 2) {
		// Use ACPI v2+ XSDT
		struct xsdt *xsdt = phys_to_virt(acpi_rsdp->xsdt_address);
		map_table(&xsdt->header);
		if (!table_checksum(&xsdt->header))
			return NULL;
		size_t num_entries = (xsdt->header.length - sizeof(struct acpi_header)) / sizeof(physaddr_t);
		for (size_t i = 0; i < num_entries; i++) {
			struct acpi_header *hd = phys_to_virt(xsdt->tables[i]);
			map_table(hd);
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
}
