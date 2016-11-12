#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define _SWAP(a, b, type) { \
		type temp = a; \
		a = b; \
		b = temp; \
	} \

COMPILER_ATTR_NORETURN
void abort(void) {
#if defined(__is_libk)
	// TODO: Add proper kernel panic.
	printf("kernel: abort()\n");
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
	printf("abort()\n");
#endif
	while (1) { }
	COMPILER_BUILTIN_UNREACHABLE();
}

static inline void _reverse(char str[], size_t length) {
	size_t start = 0;
	size_t end = length -1;
	while (start < end) {
		_SWAP(*(str+start), *(str+end), char);
		start++;
		end--;
	}
}

long int strtol(const char * restrict nptr, char ** restrict endptr, int base) {
	const char *s = nptr;
	unsigned long acc;
	int c;
	unsigned long cutoff;
	int neg = 0, any, cutlim;

	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONG_MIN : LONG_MAX;
		errno = ERANGE;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);
	return (acc);
}

char *itoa(uint32_t num, char *str, uint32_t base) {
	uint32_t i = 0;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	while (num != 0) {
		uint32_t rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
		num = num/base;
	}

	str[i] = '\0';
	_reverse(str, i);
	return str;
}
