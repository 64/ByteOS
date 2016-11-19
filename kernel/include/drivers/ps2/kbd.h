#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define KBD_CACHE_BITS 32
#define KBD_CAPSLOCK_MASK (1 << 0)
#define KBD_SCROLLLOCK_MASK (1 << 1)
#define KBD_NUMLOCK_MASK (1 << 2)

#define KBD_LEFT_ARROW (1 << 0)
#define KBD_RIGHT_ARROW (1 << 1)
#define KBD_UP_ARROW (1 << 2)
#define KBD_DOWN_ARROW (1 << 3)

struct key_modifiers {
	bool shift;
	bool control;
	bool capslock;
	bool alt;
};

struct kbd_state {
	uint32_t cache[8];
	uint8_t persist;
	uint8_t arrow_keys;
};

enum kbd_wait_state {
	KBD_STATE_NORMAL,
	KBD_STATE_MULTI_RECVD
};

void keyboard_init(void);
void keyboard_set_key(size_t index);
void keyboard_clear_key(size_t index);
bool keyboard_test_key(size_t index);
