#pragma once

#include <stdint.h>
#include <stdbool.h>

#define _VGA_COLOR_GET_FOREGROUND(c) ((c) & 0x0F)
#define _VGA_COLOR_GET_BACKGROUND(c) ((c) & 0xF0)
#define _VGA_IS_VALID_COLOR_CHAR(c) ((c) >= '0' && (c) <= '7')

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

enum vga_half_color {
	VGA_HCOLOR_BLACK = 0,
	VGA_HCOLOR_BLUE = 1,
	VGA_HCOLOR_GREEN = 2,
	VGA_HCOLOR_CYAN = 3,
	VGA_HCOLOR_RED = 4,
	VGA_HCOLOR_MAGENTA = 5,
	VGA_HCOLOR_YELLOW = 6,
	VGA_HCOLOR_WHITE = 7,
};

static inline enum vga_color vga_color_half_to_full(enum vga_half_color c, bool brightness) {
	return brightness ? (enum vga_color)(c + 8) : (enum vga_color)(c);
}

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

static inline uint8_t vga_color_get_inverse(uint8_t color) {
	uint8_t fg = _VGA_COLOR_GET_FOREGROUND(color);
	uint8_t bg = _VGA_COLOR_GET_BACKGROUND(color);
	return vga_entry_color(bg, fg);
}

static inline enum vga_half_color vga_parse_color(char input) {
	enum vga_half_color returner;
	switch(input) {
		case '0':
			returner = VGA_HCOLOR_BLACK;
			break;
		case '1':
			returner = VGA_HCOLOR_RED;
			break;
		case '2':
			returner = VGA_HCOLOR_GREEN;
			break;
		case '3':
			returner = VGA_HCOLOR_YELLOW;
			break;
		case '4':
			returner = VGA_HCOLOR_BLUE;
			break;
		case '5':
			returner = VGA_HCOLOR_MAGENTA;
			break;
		case '6':
			returner = VGA_HCOLOR_CYAN;
			break;
		case '7':
		case '9':
		default:
			returner = VGA_HCOLOR_WHITE;
			break;
	};
	return returner;
}
