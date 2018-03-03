#pragma once

#include <stdbool.h>
#include <stdint.h>

void pit_init(void);
void pit_sleep_ms(uint64_t);
void pit_sleep_watch_flag(uint64_t, volatile bool *, bool);
