#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define KBD_CACHE_BITS 32
#define KBD_CAPSLOCK_MASK (1 << 0)
#define KBD_SCROLLLOCK_MASK (1 << 1)
#define KBD_NUMLOCK_MASK (1 << 2)

typedef struct {
	bool shift;
	bool control;
	bool capslock;
	bool alt;
} key_modifiers;

typedef struct {
	uint32_t cache[8];
	uint8_t persist;
} kbd_state;

void keyboard_install(void);
void keyboard_set_key(size_t index);
void keyboard_clear_key(size_t index);
bool keyboard_test_key(size_t index);
