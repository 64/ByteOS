#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PS2_DATA 0x60
#define PS2_COMMAND 0x64
#define PS2_STATUS 0x64

#define PS2_STATUS_OUTBUF (1 << 0)
#define PS2_STATUS_INBUF (1 << 1)
#define PS2_STATUS_SYSFLAG (1 << 2)
#define PS2_STATUS_DATA (1 << 3)
#define PS2_STATUS_UNKNOWN1 (1 << 4)
#define PS2_STATUS_UNKNOWN2 (1 << 5)
#define PS2_STATUS_TIMEOUT_ERR (1 << 6)
#define PS2_STATUS_PARITYERR_ERR (1 << 7)

#define PS2_CONFIG_PORT1_INT (1 << 0)
#define PS2_CONFIG_PORT2_INT (1 << 1)
#define PS2_CONFIG_SYSFLAG (1 << 2)
#define PS2_CONFIG_ZERO1 (1 << 3)
#define PS2_CONFIG_PORT1_CLOCK (1 << 4)
#define PS2_CONFIG_PORT2_CLOCK (1 << 5)
#define PS2_CONFIG_PORT1_TRANSLATION (1 << 6)
#define PS2_CONFIG_ZERO2 (1 << 7)

#define PS2_RES_PORT_TEST_PASS 0x00
#define PS2_RES_CONTROLLER_TEST_PASS 0x55
#define PS2_RES_CONTROLLER_TEST_FAIL 0xFC

#define PS2_CMD_READ_CONFIG 0x20
#define PS2_CMD_WRITE_CONFIG 0x60
#define PS2_CMD_READN(n) ((n) + 0x20)
#define PS2_CMD_WRITEN(n) ((n) + 0x61)
#define PS2_CMD_PORT1_DISABLE 0xAD
#define PS2_CMD_PORT1_ENABLE 0xAE
#define PS2_CMD_PORT1_TEST 0xAB
#define PS2_CMD_PORT2_DISABLE 0xA7
#define PS2_CMD_PORT2_ENABLE 0xA8
#define PS2_CMD_PORT2_TEST 0xA9
#define PS2_CMD_CONTROLLER_TEST 0xAA
#define PS2_CMD_PORT2_WRITE 0xD4

#define PS2_DEVICE_RESET 0xFF

void ps2_init();
void ps2_wait_input();
void ps2_wait_output();
void ps2_port1_write(uint8_t data);
void ps2_port2_write(uint8_t data);
uint8_t ps2_port_read();
void ps2_cmd_write(uint8_t cmd);
