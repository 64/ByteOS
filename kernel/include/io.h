#pragma once
#include <stdint.h>

static inline void io_outportb(uint16_t port, uint8_t val) {
	asm volatile (
		"outb %0, %1"
		:
		: "a"(val), "Nd"(port)
	);
}

static inline uint8_t io_inportb(uint16_t port) {
	uint8_t ret;
	asm volatile (
		"inb %1, %0"
		: "=a"(ret)
		: "Nd"(port)
	);
	return ret;
}
