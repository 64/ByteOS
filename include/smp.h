#pragma once

#include <stdint.h>

#include "interrupts.h"

uint8_t smp_cpu_id(void);
void smp_init(void);
void smp_ap_kmain(void);

void ipi_abort(struct isr_context *regs);
