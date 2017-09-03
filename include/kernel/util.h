#pragma once

#define MIN(x, y) ({ \
	const typeof((x)) _x = (x); \
	const typeof((y)) _y = (y); \
	_x > _y ? _y : _x; })

#define MAX(x, y) ({ \
	const typeof((x)) _x = (x); \
	const typeof((y)) _y = (y); \
	_x > _y ? _x : _y; })

#define ALIGNUP(val, align) ({ \
	const typeof((val)) _val = (val); \
	const typeof((align)) _align = (align); \
	(_val + (_align - 1)) & -_align; })

#define ISALIGN_POW2(val, align) ({ \
	const typeof((val)) _val = (val); \
	const typeof((align)) _align = (align); \
	(_val & (_align - 1)) & (_val - 1) == 0; })

#define UNUSED(x) __attribute__((unused)) x
