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

#define ALIGNDOWN(val, align) ({ \
	const typeof((val)) _val = (val); \
	const typeof((align)) _align = (align); \
	_val & ~(_align - 1); })

#define DIV_ROUND_UP(val, div) (((val) + (div) - 1) / (div))

#define ISALIGN_POW2(val, align) ({ \
	const typeof((val)) _val = (val); \
	const typeof((align)) _align = (align); \
	(_val & (_align - 1)) == 0; })

#define UNUSED(x) x __attribute__((unused))

void __attribute__((error("Static assertion failed"))) __error_fn(void);
#define _static_assert(cond) ({ \
	do { \
		if (!(cond)) \
			__error_fn(); \
	} while (0); })
