#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <tty.h>
#include <io.h>

#include <drivers/vga.h>

#define _ANSI_MAX_SEQUENCE_LENGTH 6
#define _ANSI_IS_SEQUENCE_DELIM(c) ((c) == 'm' || (c) == ';')
#define MAX(x, y) ((x) > (y) ? (x) : (y))

enum _vga_textmode_putchar_states {
	_T_STATE_WRITING,
	_T_STATE_READING
};

struct _ansi_sequence { char data[_ANSI_MAX_SEQUENCE_LENGTH + 1]; };
struct _ansi_styles {
	bool bold;
	bool italic;
	bool underline;
	bool inverse;
	uint8_t color;
}; // TODO: Make this more efficient

static struct _ansi_styles current_style;
static struct _ansi_sequence sequence_buffer;

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t vga_textmode_row;
static size_t vga_textmode_column;
static uint16_t* vga_textmode_buffer;

void vga_textmode_shiftscreen();

static inline void _vga_textmode_reset_styles(void) {
	current_style.bold = 0;
	current_style.italic = 0;
	current_style.inverse = 0;
	current_style.underline = 0;
	current_style.color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void vga_textmode_setcursor(size_t x, size_t y) {
	uint16_t position = (y * VGA_WIDTH) + x;
	io_outportb(0x3D4, 0x0F);
	io_outportb(0x3D5, (uint8_t)(position & 0xFF));
	io_outportb(0x3D4, 0x0E);
	io_outportb(0x3D5, (uint8_t)((position>>8) & 0xFF));
}

void vga_textmode_initialize(void) {
	vga_textmode_row = 0;
	vga_textmode_column = 0;
	_vga_textmode_reset_styles();
	vga_textmode_setcursor(vga_textmode_column, vga_textmode_row);
	vga_textmode_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			vga_textmode_buffer[index] = vga_entry(' ', current_style.color);
		}
	}
}

void vga_textmode_clearscreen(void) {
	memset(vga_textmode_buffer, 0x0, sizeof(uint16_t) * VGA_WIDTH * VGA_HEIGHT);
}

void vga_textmode_setcolor(uint8_t color) {
	current_style.color = color;
}

void vga_textmode_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	vga_textmode_buffer[index] = vga_entry(c, color);
}

static inline unsigned char vga_textmode_getentryat(size_t x, size_t y) {
	return vga_textmode_buffer[y * VGA_WIDTH + x];
}

void vga_textmode_shiftscreen(void) {
	size_t current_row = 0;
	while (current_row++ < VGA_HEIGHT) {
		memcpy(&VGA_MEMORY[(current_row - 1) * VGA_WIDTH], &VGA_MEMORY[current_row * VGA_WIDTH], VGA_WIDTH * sizeof(uint16_t));
	}
}

static inline void vga_textmode_addrow(void) {
	if (++vga_textmode_row == VGA_HEIGHT) {
		vga_textmode_shiftscreen();
		vga_textmode_row--;
	}
	vga_textmode_setcursor(vga_textmode_column, vga_textmode_row);
}

static inline void vga_textmode_addcol(void) {
	if (++vga_textmode_column == VGA_WIDTH) {
		vga_textmode_column = 0;
		vga_textmode_addrow();
	}
	vga_textmode_setcursor(vga_textmode_column, vga_textmode_row);
}

static inline void _vga_textmode_display_char(char c) {
	switch (c) {
		case '\n':
			vga_textmode_column = 0;
			vga_textmode_addrow();
			break;
		case '\t':
			do {
				vga_textmode_addcol();
			} while (vga_textmode_column % 4 != 0);
			break;
		case '\b':
			if (vga_textmode_column != 0) vga_textmode_column--;
			vga_textmode_setcursor(vga_textmode_column, vga_textmode_row);
			break;
		default:
			vga_textmode_putentryat(c, current_style.color, vga_textmode_column, vga_textmode_row);
			vga_textmode_addcol();
	}
}

void vga_textmode_writeb(const char *, size_t);

void _vga_textmode_set_bold(bool value) {
	current_style.bold = value;
	enum vga_color foreground = vga_color_half_to_full(_VGA_COLOR_GET_FOREGROUND(current_style.color), value);
	current_style.color = vga_entry_color(foreground, _VGA_COLOR_GET_BACKGROUND(current_style.color));
}

void _vga_textmode_set_underline(bool value) {
	(void)value; // TODO: Implement this
}

void _vga_textmode_set_inverse(bool value) {
	if (current_style.inverse != value) {
		current_style.color = vga_color_get_inverse(current_style.color);
		current_style.inverse = value;
	}
}

#define _ANSI_SEQUENCE_OFFSET(entry) (isBeginning ? (seq.data[2 + entry]) : (seq.data[entry]))
static inline void _vga_textmode_execute_ansi(struct _ansi_sequence seq, size_t size, bool isBeginning) {
	if (isBeginning && size < 4) {
		vga_textmode_writeb(seq.data, size);
		return;
	} else if (!isBeginning && size < 2) {
		vga_textmode_writeb(seq.data, size);
		return;
	}
	bool fail = 0;
	switch(_ANSI_SEQUENCE_OFFSET(0)) {
		case '0':
			if (_ANSI_IS_SEQUENCE_DELIM(_ANSI_SEQUENCE_OFFSET(1))) {
				_vga_textmode_reset_styles();
			} else
				fail = 1;
			break;
		case '1':
			if (_ANSI_IS_SEQUENCE_DELIM(_ANSI_SEQUENCE_OFFSET(1))) {
				_vga_textmode_set_bold(1);
			} else
				fail = 1;
			break;
		case '2':
			if (!_ANSI_IS_SEQUENCE_DELIM(_ANSI_SEQUENCE_OFFSET(1))) {
				switch (_ANSI_SEQUENCE_OFFSET(1)) {
					case '2': _vga_textmode_set_bold(0); break;
					case '4': _vga_textmode_set_underline(0); break;
					case '3': case '5': _vga_textmode_set_inverse(0); break; // Treat italic as inverse
					default: fail = 1; break;
				};
			} else
				fail = 1;
			break;
		case '3':
			if (!_ANSI_IS_SEQUENCE_DELIM(_ANSI_SEQUENCE_OFFSET(1)) && _VGA_IS_VALID_COLOR_CHAR(_ANSI_SEQUENCE_OFFSET(1))) {
				uint8_t half_color = vga_parse_color(_ANSI_SEQUENCE_OFFSET(1));
				uint8_t foreground = vga_color_half_to_full(half_color, current_style.bold);
				current_style.color = vga_entry_color(foreground, _VGA_COLOR_GET_BACKGROUND(current_style.color));
			} else
				fail = 1;
			break;
		case '4':
			if (!_ANSI_IS_SEQUENCE_DELIM(_ANSI_SEQUENCE_OFFSET(1)) && _VGA_IS_VALID_COLOR_CHAR(_ANSI_SEQUENCE_OFFSET(1))) {
				uint8_t half_color = vga_parse_color(_ANSI_SEQUENCE_OFFSET(1));
				uint8_t background = vga_color_half_to_full(half_color, current_style.bold);
				current_style.color = vga_entry_color(_VGA_COLOR_GET_FOREGROUND(current_style.color), background);
			} else
				fail = 1;
			break;
		case '7':
			if (_ANSI_IS_SEQUENCE_DELIM(_ANSI_SEQUENCE_OFFSET(1))) {
				_vga_textmode_set_inverse(1);
			} else
				fail = 1;
			break;
		default:
			fail = 1;
			break;
	}
	if (fail) vga_textmode_writeb(seq.data, size);
}

void _vga_textmode_putchar_state(char c) {
	static enum _vga_textmode_putchar_states state = _T_STATE_WRITING;
	static int seqbuf_i = 0;
	if (state == _T_STATE_WRITING) {
		if (c == (char)27) {
			seqbuf_i = 0;
			sequence_buffer.data[seqbuf_i++] = (char)27;
			state = _T_STATE_READING;
		} else
			_vga_textmode_display_char(c);
	} else if (state == _T_STATE_READING) {
		if (!_ANSI_IS_SEQUENCE_DELIM(c) && seqbuf_i < _ANSI_MAX_SEQUENCE_LENGTH) {
			sequence_buffer.data[seqbuf_i++] = c;
		} else if (c == ';' && seqbuf_i < _ANSI_MAX_SEQUENCE_LENGTH) {
			sequence_buffer.data[seqbuf_i++] = c;
			_vga_textmode_execute_ansi(sequence_buffer, seqbuf_i, (sequence_buffer.data[0] == 27 ? 1 : 0));
			seqbuf_i = 0;
		} else if (c == 'm' && seqbuf_i < _ANSI_MAX_SEQUENCE_LENGTH) {
			sequence_buffer.data[seqbuf_i++] = c;
			_vga_textmode_execute_ansi(sequence_buffer, seqbuf_i, (sequence_buffer.data[0] == 27 ? 1 : 0));
			seqbuf_i = 0;
			state = _T_STATE_WRITING;
		} else {
			sequence_buffer.data[seqbuf_i++] = c;
			vga_textmode_writeb(sequence_buffer.data, seqbuf_i);
			seqbuf_i = 0;
			state = _T_STATE_WRITING;
		}
	}
}

void vga_textmode_putchar(char c) {
	_vga_textmode_putchar_state(c);
}

void vga_textmode_writeb(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++)
		_vga_textmode_display_char(data[i]);
}

void vga_textmode_write(const char *data, size_t size) {
	for (size_t i = 0; i < size; i++)
		vga_textmode_putchar(data[i]);
}

void vga_textmode_writestring(const char *data) {
	vga_textmode_write(data, strlen(data));
}
