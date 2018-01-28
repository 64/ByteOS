#include <stddef.h>
#include <stdint.h>

#include "drivers/cansid.h"

#define ESC '\x1B'

struct cansid_state cansid_init(void)
{
	struct cansid_state rv = {
		.state = CANSID_ESC,
		.style = 0x0F,
		.next_style = 0x0F
	};
	return rv;
}

static inline unsigned char cansid_convert_color(unsigned char color)
{
	const unsigned char lookup_table[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
	return lookup_table[(int)color];
}

struct color_char cansid_process(struct cansid_state *state, char x)
{
	struct color_char rv = {
		.style = state->style,
		.ascii = '\0'
	};
	switch (state->state) {
		case CANSID_ESC:
			if (x == ESC)
				state->state = CANSID_BRACKET;
			else {
				rv.ascii = x;
			}
			break;
		case CANSID_BRACKET:
			if (x == '[')
				state->state = CANSID_PARSE;
			else {
				state->state = CANSID_ESC;
				rv.ascii = x;
			}
			break;
		case CANSID_PARSE:
			if (x == '3') {
				state->state = CANSID_FGCOLOR;
			} else if (x == '4') {
				state->state = CANSID_BGCOLOR;
			} else if (x == '0') {
				state->state = CANSID_ENDVAL;
				state->next_style = 0x0F;
			} else if (x == '1') {
				state->state = CANSID_ENDVAL;
				state->next_style |= (1 << 3);
			} else if (x == '=') {
				state->state = CANSID_EQUALS;
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
		case CANSID_BGCOLOR:
			if (x >= '0' && x <= '7') {
				state->state = CANSID_ENDVAL;
				state->next_style &= 0x1F;
				state->next_style |= cansid_convert_color(x - '0') << 4;
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
		case CANSID_FGCOLOR:
			if (x >= '0' && x <= '7') {
				state->state = CANSID_ENDVAL;
				state->next_style &= 0xF8;
				state->next_style |= cansid_convert_color(x - '0');
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
		case CANSID_EQUALS:
			if (x == '1') {
				state->state = CANSID_ENDVAL;
				state->next_style &= ~(1 << 3);
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
		case CANSID_ENDVAL:
			if (x == ';') {
				state->state = CANSID_PARSE;
			} else if (x == 'm') {
				// Finish and apply styles
				state->state = CANSID_ESC;
				state->style = state->next_style;
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
		default:
			break;
	}
	return rv;
}
