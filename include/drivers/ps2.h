#pragma once

#include <stdint.h>
#include "asm.h"
#include "libk.h"

#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64
#define PS2_DATA 0x60

void ps2kbd_init(void);

void ps2_init(void);

static inline uint8_t ps2_read_data(void)
{
	while ((inb(PS2_STATUS) & 1) == 0)
		;
	return inb(PS2_DATA);
}

static inline uint8_t ps2_read_status(void)
{
	return inb(PS2_STATUS);
}

static inline void ps2_write_cmd(uint8_t data)
{
	while ((inb(PS2_STATUS) & 2) != 0)
		;
	outb(PS2_COMMAND, data);
}

static inline void ps2_write_data(uint8_t data)
{
	while ((inb(PS2_STATUS) & 2) != 0)
		;
	outb(PS2_DATA, data);
}

static inline uint8_t ps2_read_config(void)
{
	ps2_write_cmd(0x20);
	uint8_t config_byte = ps2_read_data();
	kassert_dbg((config_byte & (1 << 7)) == 0);
	return config_byte;
}

static inline void ps2_write_config(uint8_t config_byte)
{
	ps2_write_cmd(0x60);
	ps2_write_data(config_byte);
}

