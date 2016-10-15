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

static inline void io_outportw(uint16_t port, uint16_t val) {
	asm volatile (
		"outw %0, %1"
		:
		: "a"(val), "Nd"(port)
	);
}

static inline uint16_t io_inportw(uint16_t port) {
	uint16_t ret;
	asm volatile (
		"inw %1, %0"
		: "=a"(ret)
		: "Nd"(port)
	);
	return ret;
}
