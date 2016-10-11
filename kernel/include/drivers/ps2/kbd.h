#pragma once

#include <stdint.h>

typedef struct {
	uint16_t data[8];
} kbd_state;

void keyboard_install(void);
