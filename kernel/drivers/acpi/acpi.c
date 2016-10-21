#include <drivers/acpi.h>
#include <drivers/pit.h>
#include <interrupt.h>
#include <io.h>
#include <memory/memory.h>
#include <string.h>
#include <klog.h>

struct acpi_rsdt_header *rsdt = NULL;
struct acpi_fadt *fadt = NULL;
struct acpi_dsdt *dsdt = NULL;
struct acpi_info acpi_info;
uint32_t rsdt_entries = 0;
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
	memset(&acpi_info, 0, sizeof(struct acpi_info));
	if (!acpi_find_rsdt()) {
		klog_warn("ACPI: RSDP not found!\n");
		return;
	}

	if (!acpi_check_header(&rsdt->h, "RSDT")) {
		klog_warn("ACPI: RSDT not found!\n");
		return;
	}

	rsdt_entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;
	if (!acpi_find_table("FACP", (struct acpi_table_header **)&fadt)) {
		klog_warn("ACPI: FADT not found!\n");
		return;
	}

	// Not even sure if this works or what it may do
	irq_install_handler(fadt->sci_interrupt, acpi_sci_interrupt_handler);
	dsdt = fadt->dsdt;
	acpi_info.power_management_profile = fadt->power_management_profile;

	if (!acpi_check_header(&dsdt->header, "DSDT")) {
		klog_warn("ACPI: DSDT not found!\n");
		return;
	}

	if (!acpi_parse_dsdt(&acpi_info)) {
		klog_warn("ACPI: Failed to parse DSDT.\n");
		return;
	}

	klog_notice("ACPI successfully initialized!\n");
	acpi_ready = 1;
}

bool acpi_parse_dsdt(struct acpi_info *info) {
	uint32_t dsdt_len = dsdt->header.length;
	int8_t *s5_addr = (int8_t*)&dsdt->def_block;

	while (0 < dsdt_len) {
		if (memcmp(s5_addr, "_S5_", 4) == 0)
			break;
		s5_addr++;
		dsdt_len--;
	}

	if (dsdt_len == 0)
		return 0;

	if (!((s5_addr[-1] == 0x08 || ( s5_addr[-2] == 0x08 && s5_addr[-1] == '\\'))
	 	&& s5_addr[4] == 0x12))
		return 0;

	s5_addr += 5;
	s5_addr += ((*s5_addr & 0xC0) >> 6) + 2;

	if (*s5_addr == 0x0A)
		s5_addr++;

	info->slp_typa = *(s5_addr) << 10;
	s5_addr++;

	if (*s5_addr == 0x0A)
		s5_addr++;

	info->slp_typb = *(s5_addr) << 10;
	info->slp_en = 1 << 13;

	return 1;
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
	(void)r;
	// TODO: Finish this
	klog_notice("Power off button press detected.\n");
	acpi_shutdown();
}

void acpi_shutdown(void) {
	if (!acpi_ready || !acpi_enable()) {
		klog_fatal("ACPI not initialized successfully, cannot shutdown.\n");
		return;
	}

	printf("Shutting down...\n");

	// Send shutdown command
	io_outportw((uint32_t)fadt->pm1a_control_block, acpi_info.slp_typa | acpi_info.slp_en);
	if (fadt->pm1b_control_block != 0)
		io_outportw((uint32_t)fadt->pm1b_control_block, acpi_info.slp_typb | acpi_info.slp_en);

	klog_fatal("Shutdown failed! Trying to resume normal execution...\n");
}
