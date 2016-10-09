#pragma once

#include <stdint.h>
#include <system.h>

void irq_install_handler(uint32_t, void (*)(struct regs *));
void irq_ack(uint8_t);
void irq_install(void);
void isr_install_handler(uint32_t, void (*)(struct regs *));
