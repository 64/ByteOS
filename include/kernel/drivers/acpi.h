#pragma once

#include <stdint.h>

#include "types.h"

void acpi_init(void);
virtaddr_t acpi_find_table(char *signature);

struct acpi_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

struct acpi_madt {
	struct acpi_header header;
	uint32_t lapic_address;
	uint32_t flags;
	uint8_t entries[];
} __attribute__((packed));

extern struct rsdp *acpi_rsdp;
