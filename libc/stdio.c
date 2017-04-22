#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define _ITOA_PARAMS val, buf_start, base, ucase, zero_pad, show_sign, space
#define __ITOA(VAL, BUFFER, BASE, UPPERCASE, ZPAD, FORCE, UNSIGN, SPACE) \
	char	*pbuffer = (BUFFER); \
	int	negative = 0; \
	unsigned int	i, len; \
	if ((BASE) > 16) \
		return 0; \
	if ((VAL) < 0 && !(UNSIGN)) { \
		negative = 1; \
		(VAL) = -(VAL); \
	} \
	do { \
		int digit = (VAL) % (BASE); \
		*(pbuffer++) = (digit < 10 ? '0' + digit : ((UPPERCASE) ? 'A' : 'a') + digit - 10);  \
		(VAL) /= (BASE); \
	} while ((VAL) > 0); \
	for (i = (pbuffer - (BUFFER)); i < (ZPAD); i++) \
		*(pbuffer++) = '0'; \
	if (negative) \
		*(pbuffer++) = '-'; \
	else if (FORCE) \
		*(pbuffer++) = '+'; \
	else if (SPACE) \
		*(pbuffer++) = ' '; \
	\
	*(pbuffer) = '\0'; \
	len = (pbuffer - (BUFFER)); \
	for (i = 0; i < len / 2; i++) { \
		char j = (BUFFER)[i]; \
		(BUFFER)[i] = (BUFFER)[len-i-1]; \
		(BUFFER)[len-i-1] = j; \
	} \
	return len;

#define __IS_UNSIGNED_SPEC(c) ((c) == 'u' || (c) == 'p' || (c) == 'x' || (c) == 'X')

#define DTOA_BUFSIZE 128
#define __DTOA_STYLE_F 1
#define __DTOA_STYLE_E 2
#define __DTOA_STYLE_G 3
#define __DTOA_STYLE_A 4

enum _printf_state {
	_PRINTF_STATE_NORMAL,
	_PRINTF_STATE_FLAGS,
	_PRINTF_STATE_FIELD,
	_PRINTF_STATE_PRECISION,
	_PRINTF_STATE_LENGTH,
	_PRINTF_STATE_CONVSPEC
};

enum _printf_lenmod {
	_PRINTF_LENMOD_NONE,
	_PRINTF_LENMOD_HH,
	_PRINTF_LENMOD_H,
	_PRINTF_LENMOD_L,
	_PRINTF_LENMOD_LL,
	_PRINTF_LENMOD_J,
	_PRINTF_LENMOD_Z,
	_PRINTF_LENMOD_T,
	_PRINTF_LENMOD_CAPL
};

typedef bool (*_print_func)(const char *, size_t);
typedef bool (*_flush_func)(void);

#if defined(__is_libk)
#include <tty.h>
#endif

#if 0
static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return 0;
	return 1;
}

int __safe_printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;
		} else if (*format == 'd') {
			format++;
			int d = (int)va_arg(parameters, int);
			char buffer[20];
			const char* str = itoa(d, buffer, 10);
			size_t len = strlen(str);
			if (maxrem < len) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;

		} else if (*format == 'x') {
			format++;
			int d = (int)va_arg(parameters, int);
			char buffer[20];
			const char* str = itoa(d, buffer, 16);
			size_t len = strlen(str);
			if (maxrem < len) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len + 2;

		} else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}
#endif

static int __ctoa(signed char value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 0, space);
}

static int __uctoa(unsigned char value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 1, space);
}

static int __stoa(signed short value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 0, space);
}

static int __ustoa(unsigned short value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 1, space);
}

static int __itoa(signed int value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 0, space);
}

static int __uitoa(unsigned int value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 1, space);
}

static int __ltoa(long int value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 0, space);
}

static int __ultoa(unsigned long int value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 1, space);
}

static int __lltoa(long long int value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 0, space);
}

static int __ulltoa(unsigned long long int value, char *buf, unsigned int base, bool uppercase, bool zero_pad, bool force_sign, bool space) {
	__ITOA(value, buf, base, uppercase, zero_pad, force_sign, 1, space);
}

static char *__dtoa_msgs[8] = {
	"infinity",
	"nan",
	"INFINITY",
	"NAN",
	"-infinity",
	"-nan",
	"-INFINITY",
	"-NAN"
};

int __dtoa(char *buf, double val, bool force_sign, bool zero_pad,
     bool space, bool alt_form, bool upper, int style,
     int precision, int field_width)
{

	if (val == NAN || val == INFINITY)
		style = __DTOA_STYLE_F;

	switch (style) {
		case __DTOA_STYLE_F: {
			char integer_part[64];
			int i, character_count;
			double fp_int, fp_frac, fp;
			bool negative;
		__dtoa_goto_style_f:
			precision = (precision == -1) ? 6 : precision;
			character_count = 0;
			fp = val;
			negative = 0;

			// NAN doesn't display, but I'll keep it regardless
			int msg_index = -1;
			if (val == NAN || val == -NAN)
				msg_index = (negative ? (upper ? 7 : 5) : (upper ? 3 : 1));
			else if (val == INFINITY || val == -INFINITY)
				msg_index = (negative ? (upper ? 6 : 4) : (upper ? 2 : 0));

			if (msg_index != -1) {
				strcpy(buf, __dtoa_msgs[msg_index]);
				return strlen(buf);
			}

			if (val < 0) {
				negative = 1;
				fp = -val;
			}

			fp_frac = modf(fp, &fp_int);

			if (fp_int == 0)
				integer_part[character_count++] = '0';

			while (fp_int > 0) {
				integer_part[character_count++] = '0' + (int)fmod(fp_int, 10);
				fp_int = floor(fp_int / 10);
			}

			// TODO: add zero padding
			(void)zero_pad;

			if (negative)
				integer_part[character_count++] = '-';
			else if (force_sign)
				integer_part[character_count++] = '+';
			else if (space)
				integer_part[character_count++] = ' ';

			for (i = 0; i < character_count; i++)
				buf[i] = integer_part[character_count - i - 1];

			if (alt_form || (fp_frac > 0 && precision != 0))
				buf[character_count++] = '.';

			while (fp_frac > 0 && precision--) {
				fp_frac *= 10;
				fp_frac = modf(fp_frac, &fp_int);
				buf[character_count++] = '0' + (int)fp_int;
			}
			buf[character_count] = '\0';
			return (int)(&buf[character_count] - &buf[0]);
		};

		case __DTOA_STYLE_E: {
			int character_count, exponent;
			double mantissa;
		__dtoa_goto_style_e:
			precision = (precision == -1) ? 6 : precision;
			character_count = exponent = 0;

			if (val != 0)
				exponent = (int)log10(fabs(val));

			mantissa = val / pow(10, exponent);

			if (mantissa < 0) {
				buf[character_count++] = '-';
				mantissa = -mantissa;
			} else if (force_sign)
				buf[character_count++] = '+';
			else if (space)
				buf[character_count++] = ' ';

			if (mantissa < 1)
				mantissa *= 10;
			else if (mantissa >= 10)
				while (mantissa > 9)
					mantissa /= 10;

			double int_part, frac_part = modf(mantissa, &int_part);

			buf[character_count++] = '0' + (int)int_part;
			if (!(precision == 0 && !alt_form))
				buf[character_count++] = '.';

			// TODO: Round fractional part
			while (frac_part > 0 && precision--) {
				frac_part *= 10;
				frac_part = modf(frac_part, &int_part);
				buf[character_count++] = '0' + (int)int_part;
			}

			// TODO: Add zero padding
			(void)zero_pad;

			if (precision > 0)
				while (precision--)
					buf[character_count++] = '0';

			modf(mantissa, &int_part);
			buf[character_count++] = (upper ? 'E' : 'e');

			if (exponent < 0) {
				buf[character_count++] = '-';
				exponent = -exponent;
				printf("%d\n", exponent);
			} else
				buf[character_count++] = '+';

			if (exponent == 0) {
				buf[character_count++] = '0';
				buf[character_count++] = '0';
			} else if (exponent < 9)
				buf[character_count++] = '0';

			while (exponent != 0 && precision--) {
				buf[character_count++] = '0' + (exponent % 10);
				exponent /= 10;
			}

			buf[character_count] = '\0';
			return (int)(&buf[character_count] - &buf[0]);
		};

		case __DTOA_STYLE_G: {
			if (precision == 0)
				precision = 1;
			else if (precision == -1)
				precision = 6;

			int exponent = val == 0 ? 0 : (int)log10(fabs(val));

			if (precision > exponent && exponent >= -4) {
				precision -= (exponent + 1);
				goto __dtoa_goto_style_f;
			} else {
				precision--;
				goto __dtoa_goto_style_e;
			}

			// TODO: More cleansing of input here
		};

		case __DTOA_STYLE_A: {
			return -1;
		};

		default:
			return -1;
	};
}

bool test_print(const char *data, size_t len) {
	while (len--)
		putchar(*data++);
	return 1;
}

int _print_with_pad(const char *buf, _print_func print_fn, unsigned int len, bool justify_left, unsigned int field_width, size_t max_remain) {
	unsigned int pad_left = 0, pad_right = 0, total;
	if (justify_left && len < field_width)
		pad_right = field_width - len;
	else if (len < field_width)
		pad_left = field_width - len;
	total = pad_left + len + pad_right;
	if (max_remain < (pad_left + len + pad_right))
		// Set ERRNO
		return -1;
	while (pad_left--)
		if (!print_fn(" ", 1))
			return -1;
	if (!print_fn(buf, len))
		return -1;
	while (pad_right--)
		if (!print_fn(" ", 1))
			return -1;
	return total;
}

int _base_printf(_print_func print_fn, _flush_func flush_fn, const char *fmt, va_list *args) {
	const char *s;
	enum _printf_state state = _PRINTF_STATE_NORMAL;
	enum _printf_lenmod len_mod = _PRINTF_LENMOD_NONE;
	int bytes_written = 0;
	bool justify_left = 0, zero_pad = 0, alt_form = 0, show_sign = 0, space = 0;
	unsigned int field_width = 0, precision = -1;

	for (s = fmt; *s != '\0'; s++) {
		size_t max_remain = INT_MAX - bytes_written;
		if (state == _PRINTF_STATE_FLAGS) {
			switch (*s) {
				case '-': justify_left = 1; zero_pad = 0; break;
				case '+': show_sign = 1; space = 0; break;
				case ' ': if (!show_sign) space = 1; break;
				case '#': alt_form = 1; break;
				case '0': if (!justify_left) zero_pad = 1; break;
				default:
					state = _PRINTF_STATE_FIELD;
					s--;
					break;
			};
		} else if (state == _PRINTF_STATE_FIELD) {
			if (*s == '*') {
				field_width = va_arg(*args, int);
				state = _PRINTF_STATE_PRECISION;
			} else if (isdigit(*s)) {
				char *end = (char *)s;
				field_width = strtol(s, &end, 10);
				s = end - 1;
				state = _PRINTF_STATE_PRECISION;
			} else {
				state = _PRINTF_STATE_PRECISION;
				s--;
			}
		} else if (state == _PRINTF_STATE_PRECISION) {
			if (*s == '.') {
				s++;
				if (*s == '*')
					precision = va_arg(*args, int);
				else {
					char *end = (char *)s;
					precision = strtol(s, &end, 10);
					s = end - 1;
				}
				state = _PRINTF_STATE_LENGTH;
			} else {
				state = _PRINTF_STATE_LENGTH;
				s--;
			}
		} else if (state == _PRINTF_STATE_LENGTH) {
			switch (*s) {
				case 'j': len_mod = _PRINTF_LENMOD_J; break;
				case 'z': len_mod = _PRINTF_LENMOD_Z;break;
				case 't': len_mod = _PRINTF_LENMOD_T; break;
				case 'L': len_mod = _PRINTF_LENMOD_CAPL; break;
				case 'h':
					if (s[1] == 'h' && s++)
						len_mod = _PRINTF_LENMOD_HH;
					else
						len_mod = _PRINTF_LENMOD_H;
					break;
				case 'l':
					if (s[1] == 'l' && s++)
						len_mod = _PRINTF_LENMOD_LL;
					else
						len_mod = _PRINTF_LENMOD_L;
					break;
				default:
					len_mod = _PRINTF_LENMOD_NONE;
					s--;
					break;
			};
			state = _PRINTF_STATE_CONVSPEC;
		} else if (state == _PRINTF_STATE_CONVSPEC) {
			switch (*s) {
				case 'p':
				case 'u':
				case 'i':
				case 'o':
				case 'x':
				case 'X':
				case 'd': {
					unsigned int len = 0, base = 10;
					bool ucase = (*s == 'X' || *s == 'p');
					char buf[26];
					char *buf_start = &buf[0];
					if (*s == 'o')
						base = 8;
					else if (*s == 'x' || *s == 'X' || *s == 'p')
						base = 16;
					if (alt_form || *s == 'p') {
						// TODO: Check if value is non-zero before prefixing '0x'
						buf[0] = '0';
						buf[1] = 'x';
						len = 2;
						buf_start = &buf[2];
					}
					switch (len_mod) {
						case _PRINTF_LENMOD_HH: {
							if (!__IS_UNSIGNED_SPEC(*s)) {
								signed char val = va_arg(*args, int);
								len += __ctoa(_ITOA_PARAMS);
							} else {
								unsigned char val = va_arg(*args, unsigned int);
								len += __uctoa(_ITOA_PARAMS);
							}
						}; break;
						case _PRINTF_LENMOD_H: {
							if (!__IS_UNSIGNED_SPEC(*s)) {
								short val = va_arg(*args, int);
								len += __stoa(_ITOA_PARAMS);
							} else {
								unsigned short val = va_arg(*args, unsigned int);
								len += __ustoa(_ITOA_PARAMS);
							}
						}; break;
						default:
						case _PRINTF_LENMOD_NONE:
						case _PRINTF_LENMOD_J:
						case _PRINTF_LENMOD_Z:
						case _PRINTF_LENMOD_T: {
							if (!__IS_UNSIGNED_SPEC(*s)) {
								int val = va_arg(*args, int);
								len += __itoa(_ITOA_PARAMS);
							} else {
								unsigned int val = va_arg(*args, unsigned int);
								len += __uitoa(_ITOA_PARAMS);
							}
						}; break;
						case _PRINTF_LENMOD_L: {
							if (!__IS_UNSIGNED_SPEC(*s)) {
								long val = va_arg(*args, long);
								len += __ltoa(_ITOA_PARAMS);
							} else {
								unsigned long val = va_arg(*args, unsigned long);
								len += __ultoa(_ITOA_PARAMS);
							}
						}; break;
						case _PRINTF_LENMOD_LL: {
							if (!__IS_UNSIGNED_SPEC(*s)) {
								long val = va_arg(*args, long long);
								len += __lltoa(_ITOA_PARAMS);
							} else {
								unsigned long val = va_arg(*args, unsigned long long);
								len += __ulltoa(_ITOA_PARAMS);
							}
						}; break;
					};
					len = _print_with_pad(buf, print_fn, len, justify_left, field_width, max_remain);
					bytes_written += len;
					state = _PRINTF_STATE_NORMAL;
				}; break;

				case 's': {
					char *str = va_arg(*args, char *);
					unsigned int len = strlen(str);
					len = _print_with_pad(str, print_fn, len, justify_left, field_width, max_remain);
					bytes_written += len;
					state = _PRINTF_STATE_NORMAL;
				}; break;

				case 'c': {
					unsigned int c = va_arg(*args, int), len;
					len = _print_with_pad((const char *)&c, print_fn, 1, justify_left, field_width, max_remain);
					bytes_written += len;
					state = _PRINTF_STATE_NORMAL;
				}; break;

				case 'F':
				case 'f': {
					int len;
					char buf[128];

					switch (len_mod) {
						case _PRINTF_LENMOD_CAPL: {
							long double d = va_arg(*args, long double);
							len = __dtoa(
								buf, d, show_sign, zero_pad,
								space, alt_form, (*s == 'f') ? 0 : 1,
								__DTOA_STYLE_F, precision, field_width
							);
						} break;
						default: {
							double d = va_arg(*args, double);
							len = __dtoa(
								buf, d, show_sign, zero_pad,
								space, alt_form, (*s == 'f') ? 0 : 1,
								__DTOA_STYLE_F, precision, field_width
							);
						}; break;
					};

					len = _print_with_pad(buf, print_fn, len, justify_left, field_width, max_remain);
					bytes_written += len;
					state = _PRINTF_STATE_NORMAL;
				}; break;

				case 'E':
				case 'e': {
					int len;
					char buf[128];

					switch (len_mod) {
						case _PRINTF_LENMOD_CAPL: {
							long double d = va_arg(*args, long double);
							len = __dtoa(
								buf, d, show_sign, zero_pad,
								space, alt_form, (*s == 'e') ? 0 : 1,
								__DTOA_STYLE_E, precision, field_width
							);
						} break;
						default: {
							double d = va_arg(*args, double);
							len = __dtoa(
								buf, d, show_sign, zero_pad,
								space, alt_form, (*s == 'e') ? 0 : 1,
								__DTOA_STYLE_E, precision, field_width
							);
						}; break;
					};

					len = _print_with_pad(buf, print_fn, len, justify_left, field_width, max_remain);
					bytes_written += len;
					state = _PRINTF_STATE_NORMAL;
				}; break;

				case 'G':
				case 'g': {
					int len;
					char buf[128];

					switch (len_mod) {
						case _PRINTF_LENMOD_CAPL: {
							long double d = va_arg(*args, long double);
							len = __dtoa(
								buf, d, show_sign, zero_pad,
								space, alt_form, (*s == 'g') ? 0 : 1,
								__DTOA_STYLE_G, precision, field_width
							);
						} break;
						default: {
							double d = va_arg(*args, double);
							len = __dtoa(
								buf, d, show_sign, zero_pad,
								space, alt_form, (*s == 'g') ? 0 : 1,
								__DTOA_STYLE_G, precision, field_width
							);
						}; break;
					};

					len = _print_with_pad(buf, print_fn, len, justify_left, field_width, max_remain);
					bytes_written += len;
					state = _PRINTF_STATE_NORMAL;
				}; break;

	#if 0
				case 'A':
				case 'a': {

				}; break;
	#endif

				case 'n': {
					switch (len_mod) {
						case _PRINTF_LENMOD_NONE: {
							int *p = va_arg(*args, int *);
							*p = bytes_written;
						}; break;
						case _PRINTF_LENMOD_HH: {
							signed char *p = va_arg(*args, signed char *);
							*p = bytes_written;
						}; break;
						case _PRINTF_LENMOD_H: {
							short int *p = va_arg(*args, short int *);
							*p = bytes_written;
						}; break;
						case _PRINTF_LENMOD_L: {
							long int *p = va_arg(*args, long int *);
							*p = bytes_written;
						}; break;
						case _PRINTF_LENMOD_LL: {
							long long int *p = va_arg(*args, long long int *);
							*p = bytes_written;
						}; break;
						case _PRINTF_LENMOD_J: {
							intmax_t *p = va_arg(*args, intmax_t *);
							*p = bytes_written;
						}; break;
						case _PRINTF_LENMOD_Z: {
							long int *p = va_arg(*args, long int *);
							*p = bytes_written;
						}; break;
						case _PRINTF_LENMOD_T: {
							ptrdiff_t *p = va_arg(*args, ptrdiff_t *);
							*p = bytes_written;
						}; break;
						// Technically not in the standard, but I'll add it anyway
						case _PRINTF_LENMOD_CAPL: {
							long double *p = va_arg(*args, long double *);
							*p = bytes_written;
						}; break;
					};
					state = _PRINTF_STATE_NORMAL;
				}; break;

				case '%': {
					if (max_remain < 1)
						// Set ERRNO
						return -1;
					if (!print_fn(s, 1))
						return -1;
					bytes_written++;
					state = _PRINTF_STATE_NORMAL;
				}; break;

				default:
					return -1;
					break;
			};
		} else if (state == _PRINTF_STATE_NORMAL) {
			if (*s == '%') {
				field_width = justify_left = show_sign = space = alt_form = zero_pad = 0;
				precision = -1;
				len_mod = _PRINTF_LENMOD_NONE;
				state = _PRINTF_STATE_FLAGS;
			} else {
				print_fn(s, 1);
				bytes_written++;
			}
		}
	}

	if (flush_fn)
		flush_fn();

	return bytes_written;
}

//typedef bool (*_print_func)(const char *, size_t);
//typedef bool (*_flush_func)(void);

char *__sprintf_current_str = NULL;
char *__snprintf_current_str = NULL;
size_t __snprintf_max = 0;

static bool __sprintf_print(const char *s, size_t len) {
	if (__sprintf_current_str == NULL || s == NULL)
		return 0;
	while (len--)
		*__sprintf_current_str++ = *s++;
	return 1;
}

static bool __snprintf_print(const char *s, size_t len) {
	if (__snprintf_max == 0)
		return 1;
	if (__sprintf_current_str == NULL || s == NULL)
		return 0;
	while (len-- && __snprintf_max--)
		*__sprintf_current_str++ = *s++;
	return 1;
}

static bool __printf_print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	size_t i;
	for (i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return 0;
	return 1;
}

int printf(const char * restrict format, ...) {
	va_list args;
	va_start(args, format);
	int rv = _base_printf(__printf_print, NULL, format, &args);
	va_end(args);
	return rv;
}

int sprintf(char * restrict s, const char * restrict format, ...) {
	va_list args;
	va_start(args, format);
	__sprintf_current_str = s;
	int rv = _base_printf(__sprintf_print, NULL, format, &args);
	__sprintf_current_str = NULL;
	va_end(args);
	return rv;
}

int snprintf(char * restrict s, size_t n, const char * restrict format, ...) {
	va_list args;
	va_start(args, format);
	__sprintf_current_str = s;
	__snprintf_max = n;
	int rv = _base_printf(__snprintf_print, NULL, format, &args);
	__snprintf_max = 0;
	__sprintf_current_str = NULL;
	va_end(args);
	return rv;
}

int vprintf(const char * restrict format, va_list arg) {
	return _base_printf(__printf_print, NULL, format, &arg);
}

int vsnprintf(char * restrict s, size_t n, const char * restrict format, va_list arg) {
	__sprintf_current_str = s;
	__snprintf_max = n;
	int rv = _base_printf(__snprintf_print, NULL, format, &arg);
	__snprintf_max = 0;
	__sprintf_current_str = NULL;
	return rv;
}

int vsprintf(char * restrict s, const char * restrict format, va_list arg) {
	__sprintf_current_str = s;
	int rv = _base_printf(__sprintf_print, NULL, format, &arg);
	__sprintf_current_str = NULL;
	return rv;
}


int putchar(int ic) {
#if defined(__is_libk)
	char c = (char) ic;
	vga_textmode_putchar(c);
#else
	// TODO: Implement stdio and the write system call.
#endif
	return ic;
}

int puts(const char* string) {
	return printf("%s\n", string);
}
