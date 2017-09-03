#include <stddef.h>

#include "drivers/vga_tmode.h"
#include "drivers/cansid.h"
#include "drivers/serial.h"
#include "io.h"
#include "libk.h"

const size_t VGAWIDTH = 80;
const size_t VGAHEIGHT = 25;

static uint16_t * const VGABUF = (uint16_t *)0xFFFFFFFF800B8000;
static size_t x_pos, y_pos;
static struct cansid_state cansid_state;

void vga_tmode_init(void);

void vga_tmode_init(void) {
	cansid_state = cansid_init();
	serial_init();
	vga_tmode_puts("\x1B[=1;30;47m                                     ByteOS");
	vga_tmode_puts("                                     \x1B[0m");
	kprintf("VGA textmode initialized\n");
}

static void vga_tmode_newline(void) {
	if (y_pos == VGAHEIGHT - 1) {
		// Shift every line of input upwards
		for (size_t i = 2; i < VGAHEIGHT; i++) {
			memcpy(VGABUF + ((i - 1) * VGAWIDTH), VGABUF + (i * VGAWIDTH), VGAWIDTH * 2);
		}
		// Clear bottom line
		size_t offset = VGAWIDTH * (VGAHEIGHT - 1);
		for (size_t i = 0; i < VGAWIDTH; i++)
			VGABUF[offset + i] = (0x0F << 8) | ' ';
		x_pos = 0;
	} else {
		x_pos = 0;
		y_pos++;
	}
	vga_tmode_setcursor(x_pos, y_pos);
}

static inline void vga_tmode_write_char(char c, unsigned char style, size_t x, size_t y) {
	volatile uint16_t *p = VGABUF + ((y * 80) + x);
	*p = (style << 8) | c;
}

void vga_tmode_putchar(char c) {
	struct color_char ch = cansid_process(&cansid_state, c);
	vga_tmode_raw_putchar(ch.ascii, ch.style);
}

void vga_tmode_raw_putchar(char c, unsigned char style) {
	switch (c) {
		case '\0':
			break;
		case '\n':
			vga_tmode_newline();
			break;
		case '\t':
			vga_tmode_putchar(' ');
			vga_tmode_putchar(' ');
			vga_tmode_putchar(' ');
			vga_tmode_putchar(' ');
			break;
		default: {
			vga_tmode_write_char(c, style, x_pos, y_pos);
			if (x_pos == VGAWIDTH - 1)
				vga_tmode_newline();
			else
				x_pos++;
		};
	}
	vga_tmode_setcursor(x_pos, y_pos);
}

void vga_tmode_puts(char *s) {
	while (*s)
		vga_tmode_putchar(*s++);
}

void vga_tmode_setcursor(size_t x, size_t y) {
	x_pos = x;
	y_pos = y;

	// Update the position of code on the screen
	unsigned short position = (y * 80) + x;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(position & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}
