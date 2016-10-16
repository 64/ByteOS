#include <drivers/acpi.h>
#include <io.h>
#include <string.h>
#include <klog.h>
#include <drivers/pit.h>

struct acpi_rsdt_header *rsdt;
uint32_t rsdt_entries;

bool acpi_find_rsdt(void) {
	uintptr_t iter;
	for (iter = 0x000E0000; iter < 0x000FFFFF; iter += 16) {
		struct acpi_rsdp *test = (struct acpi_rsdp*)iter;
		if (memcmp(test, "RSD PTR ", 8) == 0) {
			if ((memsum(test, sizeof(struct acpi_rsdp)) & 0xFF) == 0) {
				rsdt = test->rsdt_address;
				return 1;
			}
		}
	}
	return 0;
}

bool acpi_check_header(struct acpi_table_header *header, char *signature) {
	if (memcmp(header, signature, 4) != 0)
		return 0;
	if ((memsum(header, header->length) & 0xFF) != 0)
		return 0;
	return 1;
}

bool acpi_find_table(char *signature, struct acpi_table_header **out) {
	uint32_t iter = 0;
	klog_assert(rsdt_entries != 0 && rsdt_entries <= 25);
	struct acpi_table_header *table_header = rsdt->entries;
	for (iter = 0; iter < rsdt_entries; iter++) {
		table_header = rsdt->entries + iter;
		if (acpi_check_header(table_header, signature)) {
			*out = table_header;
			return 1;
		}
	}
	return 0;
}

void acpi_init(void) {
	if (!acpi_find_rsdt()) {
		klog_warn("RSDP not found!\n");
		return;
	}

	if (!acpi_check_header(&rsdt->h, "RSDT")) {
		klog_warn("RSDT not found!\n");
		return;
	}

	rsdt_entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;
	struct acpi_fadt *fadt = NULL;

	if (!acpi_find_table("FACP", (struct acpi_table_header **)&fadt)) {
		klog_warn("FADT not found!\n");
		return;
	}

	klog_detail("Power management profile: %s\n", fadt->power_management_profile == 1 ? "Desktop" : "Laptop");
	
}

void acpi_shutdown(void) {

}
