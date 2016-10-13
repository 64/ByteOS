// This file gets included by "drivers/ps2/kbd.c"

#include <drivers/ps2/kbd_mappings.h>




// Scancode (set 1) to keycode mappings for each key
const uint8_t keyboard_us_offsets[128] = {
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_ONE,
	KBD_K_TWO,
	KBD_K_THREE,
	KBD_K_FOUR,
	KBD_K_FIVE,
	KBD_K_SIX,
	KBD_K_SEVEN,
	KBD_K_EIGHT,
	KBD_K_NINE,
	KBD_K_ZERO,
	KBD_K_DASH,
	KBD_K_EQUALS,
	KBD_K_BACKSPACE,
	KBD_K_TAB,
	KBD_K_Q,
	KBD_K_W,
	KBD_K_E,
	KBD_K_R,
	KBD_K_T,
	KBD_K_Y,
	KBD_K_U,
	KBD_K_I,
	KBD_K_O,
	KBD_K_P,
	KBD_K_SQBRACKET_OPEN,
	KBD_K_SQBRACKET_CLOSE,
	KBD_K_ENTER,
	KBD_K_LCTRL,
	KBD_K_A,
	KBD_K_S,
	KBD_K_D,
	KBD_K_F,
	KBD_K_G,
	KBD_K_H,
	KBD_K_J,
	KBD_K_K,
	KBD_K_L,
	KBD_K_SEMICOLON,
	KBD_K_APOSTROPHE,
	KBD_K_BACKTICK,
	KBD_K_LSHIFT,
	KBD_K_BACKSLASH,
	KBD_K_Z,
	KBD_K_X,
	KBD_K_C,
	KBD_K_V,
	KBD_K_B,
	KBD_K_N,
	KBD_K_M,
	KBD_K_COMMA,
	KBD_K_PERIOD,
	KBD_K_SLASH,
	KBD_K_RSHIFT,
	KBD_K_NONE, // Keypad *
	KBD_K_LALT,
	KBD_K_SPACE,
	KBD_K_CAPS,
	KBD_K_NONE, // F1
	KBD_K_NONE, // F2
	KBD_K_NONE, // F3
	KBD_K_NONE, // F4
	KBD_K_NONE, // F5
	KBD_K_NONE, // F6
	KBD_K_NONE, // F7
	KBD_K_NONE, // F8
	KBD_K_NONE, // F9
	KBD_K_NONE, // F10
	KBD_K_NONE, // Num lock
	KBD_K_NONE, // Scroll lock
	KBD_K_NONE, // Keypad 7
	KBD_K_NONE, // Keypad 8
	KBD_K_NONE, // Keypad 9
	KBD_K_NONE, // Keypad -
	KBD_K_NONE, // Keypad 4
	KBD_K_NONE, // Keypad 5
	KBD_K_NONE, // Keypad 6
	KBD_K_NONE, // Keypad +
	KBD_K_NONE, // Keypad 1
	KBD_K_NONE, // Keypad 2
	KBD_K_NONE, // Keypad 3
	KBD_K_NONE, // Keypad 0
	KBD_K_NONE, // Keypad .
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE, // F11
	KBD_K_NONE, // F12
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE,
	KBD_K_NONE
};

const uint8_t keyboard_us_lowercase[128] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	     /* 9 */
	'9', '0', '-', '=', '\b',                 	     /* Backspace */
	'\t',		                                     /* Tab */
	'q', 'w', 'e', 'r',	                             /* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	     /* Enter key */
    	0,			                             /* 29   - Control */
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
