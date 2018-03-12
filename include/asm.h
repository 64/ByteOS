#pragma once

#include <stdint.h>

static inline uint64_t msr_read(uint64_t msr)
{
	uint32_t low, high;
	asm volatile (
		"rdmsr"
		: "=a"(low), "=d"(high)
		: "c"(msr)
	);
	return ((uint64_t)high << 32) | low;
}

static inline void msr_write(uint64_t msr, uint64_t value)
{
	uint32_t low = value & 0xFFFFFFFF;
	uint32_t high = value >> 32;
	asm volatile (
		"wrmsr"
		:
		: "c"(msr), "a"(low), "d"(high)
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

static inline void irq_enable(void)
{
	sti();
}

static inline void irq_disable(void)
{
	cli();
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

static inline void reload_cr3(void)
{
	asm volatile (
		"mov %%cr3, %%rax\n"
		"mov %%rax, %%cr3"
		:
		:
		: "rax", "memory"
	);
}

static inline void change_cr3(uintptr_t cr3)
{
	asm volatile (
		"mov %0, %%rax\n"
		"mov %%rax, %%cr3"
		:
		: "r"(cr3)
		: "rax", "memory"
	);
}

static inline void bochs_magic(void)
{
	asm volatile (
		"xchg %%bx, %%bx"
		:
		:
		: "rbx"
	);
}

static inline uint64_t read_rflags(void)
{
	return __builtin_ia32_readeflags_u64();
}

static inline void write_rflags(uint64_t rflags)
{
	__builtin_ia32_writeeflags_u64(rflags);
}
