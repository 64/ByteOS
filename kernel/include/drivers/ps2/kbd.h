#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
	uint32_t cache[8];
} kbd_state;

#define KBD_CACHE_BITS 32

void keyboard_install(void);
void keyboard_set_key(size_t index);
void keyboard_clear_key(size_t index);
bool keyboard_test_key(size_t index);
