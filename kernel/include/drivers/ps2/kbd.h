#pragma once

#include <stdint.h>

#define KEY_SHIFT (1 << 0)
#define KEY_ALT (1 << 1)
#define KEY_CTRL (1 << 2)

typedef struct {
    uint8_t modifiers;
} keyboard_state;

void keyboard_install(void);
