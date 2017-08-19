#pragma once

#include <stddef.h>
#include <stdint.h>

void vga_tmode_putchar(char);
void vga_tmode_raw_putchar(char, unsigned char);
void vga_tmode_puts(char *);
void vga_tmode_setcursor(size_t x, size_t y);
