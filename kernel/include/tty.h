#pragma once

#include <stddef.h>

void vga_textmode_initialize(void);
void vga_textmode_putchar(char c);
void vga_textmode_write(const char *data, size_t size);
void vga_textmode_writestring(const char *data);
void vga_textmode_clearscreen();
