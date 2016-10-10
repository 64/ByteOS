#include <drivers/ps2/kbd.h>
#include <isr.h>
#include <klog.h>
#include <io.h>

#define KEY_PENDING 0x64

static inline void keyboard_wait(void) {
	while (io_inportb(KEY_PENDING) & 2)
		;
}

const uint8_t keyboard_us_lowercase[128] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	     /* 9 */
	'9', '0', '-', '=', '\b',                 	     /* Backspace */
	'\t',		                                     /* Tab */
	'q', 'w', 'e', 'r',	                             /* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	     /* Enter key */
    0,			                                     /* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    /* 39 */
	'\'', '`',   0,		                             /* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',		     /* 49 */
	'm', ',', '.', '/',   0,			     /* Right shift */
	'*',
	0,	                                             /* Alt */
	' ',	                                             /* Space bar */
	0,	                                             /* Caps lock */
	0,	                                             /* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	                                             /* < ... F10 */
	0,	                                             /* 69 - Num lock*/
	0,	                                             /* Scroll Lock */
	0,	                                             /* Home key */
	0,	                                             /* Up Arrow */
	0,	                                             /* Page Up */
	'-',
	0,	                                             /* Left Arrow */
	0,
	0,	                                             /* Right Arrow */
	'+',
	0,	                                             /* 79 - End key*/
	0,	                                             /* Down Arrow */
	0,	                                             /* Page Down */
	0,	                                             /* Insert Key */
	0,	                                             /* Delete Key */
	0,   0,   0,
	0,	                                             /* F11 Key */
	0,	                                             /* F12 Key */
	0,	                                             /* All other keys are undefined */
};

const uint8_t keyboard_us_uppercase[128] = {
	0, 27,
	'!','@','#','$','%','^','&','*','(',')',
	'_','+','\b',
	'\t',                                                  /* Tab */
	'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
	0,                                                     /* control */
	'A','S','D','F','G','H','J','K','L',':','"', '~',
	0,                                                     /* left shift */
	'|','Z','X','C','V','B','N','M','<','>','?',
	0,                                                     /* right shift */
	'*',
	0,                                                     /* alt */
	' ',                                                   /* space */
	0,                                                     /* caps lock */
	0,                                                     /* F1 [59] */
	0, 0, 0, 0, 0, 0, 0, 0,
	0,                                                     /* ... F10 */
	0,                                                     /* 69 num lock */
	0,                                                     /* scroll lock */
	0,                                                     /* home */
	0,                                                     /* up */
	0,                                                     /* page up */
	'-',
	0,                                                     /* left arrow */
	0,
	0,                                                     /* right arrow */
	'+',
	0,                                                     /* 79 end */
	0,                                                     /* down */
	0,                                                     /* page down */
	0,                                                     /* insert */
	0,                                                     /* delete */
	0, 0, 0,
	0,                                                     /* F11 */
	0,                                                     /* F12 */
	0,                                                     /* everything else */
};

static inline uint8_t keyboard_scancode_convert(uint8_t scancode) {
	return keyboard_us_lowercase[scancode];
}

void keyboard_handler(struct regs *r) {
	keyboard_wait();
	uint8_t scancode = io_inportb(0x60);
	irq_ack(r->int_no - 32);

	if (scancode & 0x80)
		;
	else
		printf("%c", keyboard_scancode_convert(scancode));
}

void keyboard_install(void) {
	klog_notice("Initializing PS/2 keyboard driver...\n");
	irq_install_handler(1, keyboard_handler);
	uint8_t test = 0;
	while(((test = io_inportb(KEY_PENDING)) & 1) == 1) {
	   io_inportb(0x60);
	}
}