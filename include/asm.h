#pragma once

#include <stdint.h>

static inline uint64_t msr_read(uint64_t msr)
{
	uint64_t rv_low, rv_high;
	asm volatile (
		"rdmsr"
		: "=d"(rv_high), "=a"(rv_low)
		: "c"(msr)
	);
	return rv_low | ((rv_high << 32) & 0xFFFFFFFF00000000);
}

static inline void msr_write(uint64_t msr, uint64_t value)
{
	asm volatile (
		"wrmsr"
		:
		: "c"(msr), "a"(value & 0xFFFFFFFF), "d"((value >> 32) & 0xFFFFFFFF)
	);
}

static inline void outb(uint16_t port, uint8_t val)
{
	asm volatile (
	    "outb %0, %1"
	    :
	    : "a"(val), "Nd"(port)
	);
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	asm volatile (
		"inb %1, %0"
		: "=a"(ret)
		: "Nd"(port)
	);
	return ret;
}
