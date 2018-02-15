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

extern struct rsdp *acpi_rsdp;
