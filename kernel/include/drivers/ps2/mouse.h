#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <system.h>

#define MOUSE_LMB (1 << 0)
#define MOUSE_RMB (1 << 1)
#define MOUSE_MMB (1 << 2)
#define MOUSE_PORT 0x60
#define MOUSE_STATUS 0x64
#define MOUSE_WRITE 0xD4
#define MOUSE_ENABLE_AUX 0xA8
#define MOUSE_COMPAQ_STATUS 0x20
#define MOUSE_IRQ 12
#define MOUSE_ACK 0xFA
#define MOUSE_RESEND 0xFE
#define MOUSE_ERROR 0xFC

struct mouse_state {
	uint32_t x;
	uint32_t y;
	uint8_t btns;
};

void mouse_init();
void mouse_interrupt(struct interrupt_frame *);
