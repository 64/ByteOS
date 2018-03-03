#include "libk.h"
#include "util.h"
#include "asm.h"
#include "interrupts.h"
#include "drivers/pit.h"
#include "drivers/apic.h"

void pit_init(void)
{
}

void pit_sleep_ms(uint64_t ms)
{
	uint64_t total_count = 0x4A9 * ms;
	do {
		uint16_t count = MIN(total_count, 0xFFFFU);
		outb(0x43, 0x30);
		outb(0x40, count & 0xFF);
		outb(0x40, count >> 8);
		do {
			pause();
			outb(0x43, 0xE2);
		} while ((inb(0x40) & (1 << 7)) == 0);
		total_count -= count;
	} while ((total_count & ~0xFFFF) != 0);
}

void pit_sleep_watch_flag(uint64_t ms, volatile bool *flag, bool original)
{
	
	uint64_t total_count = 0x4A9 * ms;
	do {
		uint16_t count = MIN(total_count, 0xFFFFU);
		outb(0x43, 0x30);
		outb(0x40, count & 0xFF);
		outb(0x40, count >> 8);
		do {
			pause();
			if (__atomic_load_n(flag, __ATOMIC_RELAXED) != original)
				return;
			outb(0x43, 0xE2);
		} while ((inb(0x40) & (1 << 7)) == 0);
		total_count -= count;
	} while ((total_count & ~0xFFFF) != 0);
}
