#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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
