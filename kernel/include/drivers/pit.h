#pragma once

#include <stdint.h>
#include <system.h>

void pit_set_timer_phase(int16_t hz);
void pit_install(void);
void pit_handler(struct regs *r);
void pit_wait(uint32_t seconds);
void pit_wait_ms(uint32_t ms);
uint32_t pit_ticks();
