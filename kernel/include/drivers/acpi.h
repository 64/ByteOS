#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct rsd_ptr {
	uint8_t signature[8];
	uint8_t checksum;
	uint8_t oem_id[6];
	uint8_t revision;
	uint32_t *rsdt_addr;
};

struct facp {
	uint8_t signature[4];
	uint32_t length;
	uint8_t unused_one[32];
	uint32_t *dsdt;
	uint8_t unused_two[4];
	uint32_t *smi_cmd;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t unused_three[10];
	uint32_t *pm1a_cnt_blk;
	uint32_t *pm1b_cnt_blk;
	uint8_t unused_four[17];
	uint8_t pm1_cnt_len;
};

uint32_t *acpi_check_rsd_ptr(uint32_t *ptr);
uint32_t *acpi_get_rsd_ptr(void);
bool acpi_check_header(uint32_t *ptr, char *sig);
bool acpi_enable(void);
bool acpi_init(void);
void acpi_shutdown(void);
