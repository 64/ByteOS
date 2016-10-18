#include <drivers/acpi.h>
#include <drivers/pit.h>
#include <isr.h>
#include <io.h>
#include <string.h>
#include <klog.h>

struct acpi_rsdt_header *rsdt;
struct acpi_fadt *fadt;
struct acpi_dsdt *dsdt;
uint32_t rsdt_entries;
bool acpi_ready = 0;

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
		klog_warn("ACPI: RSDP not found!\n");
		return;
	}

	if (!acpi_check_header(&rsdt->h, "RSDT")) {
		klog_warn("ACPI: RSDT not found!\n");
		return;
	}

	rsdt_entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;
	fadt = NULL;
	dsdt = NULL;

	if (!acpi_find_table("FACP", (struct acpi_table_header **)&fadt)) {
		klog_warn("ACPI: FADT not found!\n");
		return;
	}

	// Not even sure if this works
	irq_install_handler(fadt->sci_interrupt, acpi_sci_interrupt_handler);

	dsdt = (struct acpi_dsdt *)fadt->dsdt;

	if (!acpi_check_header(&dsdt->header, "DSDT")) {
		klog_warn("ACPI: DSDT not found!\n");
		return;
	}

	klog_notice("ACPI successfully initialized\n");

	acpi_ready = 1;
}

bool acpi_enable() {
	if ((io_inportw(fadt->pm1a_control_block) & 1) != 0) {
		klog_warn("ACPI: ACPI was already enabled!\n");
		return 1;
	}

	if (fadt->smi_commandport == 0 || fadt->acpi_enable == 0) {
		klog_warn("ACPI: No known way to enable ACPI.\n");
		return 0;
	}

	io_outportb(fadt->smi_commandport, fadt->acpi_enable);
	uint32_t i;
	for (i = 0; i < 300; i++) {
		if ((io_inportw(fadt->pm1a_control_block) & 1) == 1)
			break;
		pit_wait_ms(10);
	}

	if (fadt->pm1b_control_block != 0) {
		for (; i < 300; i++) {
			if ((io_inportw(fadt->pm1b_control_block) & 1) == 1)
				break;
			pit_wait_ms(10);
		}
	}

	if (i > 300) {
		klog_fatal("ACPI enabling failed!\n");
		return 0;
	}
	
	return 1;
}

void acpi_sci_interrupt_handler(struct regs *r) {
	// TODO: Finish this
	(void)r;
}

void acpi_shutdown(void) {
	if (!acpi_ready || !acpi_enable()) {
		klog_fatal("ACPI not initialized successfully, cannot shutdown.\n");
		return;
	}

	printf("Shutting down...\n");
}
