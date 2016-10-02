#pragma once

#include <stdint.h>
#include <arch/i386/io.h>
#include <arch/i386/system.h>

void pit_set_timer_phase(int16_t hz);
void pit_install(void);
void pit_handler(struct regs *r);
void pit_timer_wait(uint32_t seconds);
void pit_timer_wait_ms(uint32_t ms);
uint32_t pit_ticks();

extern void irq_install_handler(uint32_t, void (*)(struct regs *));
