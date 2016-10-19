#include <drivers/pit.h>
#include <interrupt.h>
#include <io.h>
#include <klog.h>
#include <stdlib.h>

#define PIT_BASE           0x40
#define PIT_CHANNEL0       PIT_BASE
#define PIT_CHANNEL1       (PIT_BASE + 1)
#define PIT_CHANNEL2       (PIT_BASE + 2)
#define PIT_COMMAND        (PIT_BASE + 3)
#define PIT_SPEED          1193180
#define PIT_CONSTANT 1000
#define PIT_SET            0x36

volatile uint32_t pit_tick_count = 0;

void pit_set_timer_phase(int16_t hz) {
	int16_t divisor = PIT_SPEED / hz;   /* Calculate our divisor */
	io_outportb(PIT_COMMAND, PIT_SET);             /* Set our command byte 0x36 */
	io_outportb(PIT_CHANNEL0, divisor & 0xFF);   /* Set low byte of divisor */
	io_outportb(PIT_CHANNEL0, divisor >> 8);     /* Set high byte of divisor */
}

void pit_install(void) {
	pit_set_timer_phase(PIT_CONSTANT);
	irq_install_handler(0, pit_handler);
}

void pit_handler(struct regs *r) {
	if (r->int_no != 32) {
		klog_fatal("The PIT handler must be triggered by IRQ 0\n");
		abort();
	}
	++pit_tick_count;
	irq_ack(r->int_no - 32);
}

uint32_t pit_ticks() {
	return pit_tick_count;
}

void pit_wait(uint32_t seconds) {
	uint32_t desired_ticks = pit_tick_count + (seconds * PIT_CONSTANT);
	while (desired_ticks > pit_tick_count) {
		asm volatile ("sti//hlt//cli");
	}
}

void pit_wait_ms(uint32_t ms) {
	uint32_t desired_ticks = pit_tick_count + (ms * (PIT_CONSTANT / 1000));
	while (desired_ticks > pit_tick_count) {
		asm volatile ("sti//hlt//cli");
	}
}
