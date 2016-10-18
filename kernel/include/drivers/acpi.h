#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <system.h>
#include <sys/cdefs.h>

struct acpi_table_header {
	int8_t signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	int8_t oem_id[6];
	int8_t oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
};

struct acpi_rsdt_header {
	struct acpi_table_header h;
	struct acpi_table_header *entries;
	// More entries here, just we don't know how many until runtime
};

struct acpi_rsdp {
	int8_t signature[8];
	uint8_t checksum;
	int8_t oem_id[6];
	uint8_t revision;
	struct acpi_rsdt_header *rsdt_address;
};

struct acpi_generic_address {
	uint8_t address_space;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t access_size;
	uint64_t address;
} COMPILER_ATTR_PACKED;

struct acpi_fadt {
	struct acpi_table_header header;
	uint32_t firmware_ctrl;
	uint32_t dsdt;
	uint8_t  reserved; // Unused
	uint8_t  power_management_profile;
	uint16_t sci_interrupt;
	uint32_t smi_commandport;
	uint8_t  acpi_enable;
	uint8_t  acpi_disable;
	uint8_t  s4bios_req;
	uint8_t  pmstate_control;
	uint32_t pm1a_event_block;
	uint32_t pm1b_event_block;
	uint32_t pm1a_control_block;
	uint32_t pm1b_control_block;
	uint32_t pm2_control_block;
	uint32_t pm_timer_block;
	uint32_t gpe0_block;
	uint32_t gpe1_block;
	uint8_t  pm1_event_length;
	uint8_t  pm1_control_length;
	uint8_t  pm2_control_length;
	uint8_t  pm_timer_length;
	uint8_t  gpe0_length;
	uint8_t  gpe1_length;
	uint8_t  gpe1_base;
	uint8_t  cstate_control;
	uint16_t worst_c2_latency;
	uint16_t worst_c3_latency;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t  duty_offset;
	uint8_t  duty_width;
	uint8_t  duty_alarm;
	uint8_t  month_alarm;
	uint8_t  century;

	// All of the below is unused in ACPI 1.0, and can be ignored
	uint16_t boot_arch_flags;
	uint8_t  reserved_2;
	uint32_t flags;
	struct acpi_generic_address reset_reg;
	uint8_t  reset_value;
	uint8_t  reserved_3[3];
	uint64_t x_firmware_control;
	uint64_t x_dsdt;
	struct acpi_generic_address x_pm1a_event_block;
	struct acpi_generic_address x_pm1b_event_block;
	struct acpi_generic_address x_pm1a_control_block;
	struct acpi_generic_address x_pm1b_control_block;
	struct acpi_generic_address x_pm2_control_block;
	struct acpi_generic_address x_pm_timer_block;
	struct acpi_generic_address x_gpe0_block;
	struct acpi_generic_address x_gpe1_block;
};

struct acpi_dsdt {
	struct acpi_table_header header;
	uint8_t *def_block;
};

bool acpi_find_rsdt(void);
void acpi_sci_interrupt_handler(struct regs *r);
bool acpi_find_table(char *signature, struct acpi_table_header **out);
bool acpi_enable();
bool acpi_check_header(struct acpi_table_header *header, char *signature);
void acpi_init(void);
void acpi_shutdown(void);
