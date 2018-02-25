#pragma once

#include <stdint.h>

static inline uint64_t msr_read(uint64_t msr)
{
	uint64_t rv;
	asm volatile (
		"rdmsr"
		: "=A"(rv)
		: "c"(msr)
		: "rdx"
	);
	return rv;
}

static inline void msr_write(uint64_t msr, uint64_t value)
{
	asm volatile (
		"wrmsr"
		:
		: "c"(msr), "A"(value)
	);
}

static inline void invlpg(uint64_t addr)
{
	asm volatile (
		"invlpg (%0)"
		:
		: "b"(addr)
		: "memory"
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

static inline void cli(void)
{
	asm volatile (
		"cli"
		:
		:
		: "cc"
	);
}

static inline void sti(void)
{
	asm volatile (
		"sti"
		:
		:
		: "cc"
	);
}

static inline void barrier(void)
{
	asm volatile (
		""
		:
		:
		: "memory"
	);
}

static inline void pause(void)
{
	__builtin_ia32_pause();
}

static inline uint64_t read_rflags(void)
{
	return __builtin_ia32_readeflags_u64();
}

static inline void write_rflags(uint64_t rflags)
{
	__builtin_ia32_writeeflags_u64(rflags);
}
