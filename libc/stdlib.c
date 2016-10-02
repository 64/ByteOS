#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#define _SWAP(a, b, type) { \
            type temp = a; \
            a = b; \
            b = temp; \
        } \

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
	// TODO: Add proper kernel panic.
	printf("kernel: panic: abort()\n");
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
	printf("abort()\n");
#endif
	while (1) { }
	__builtin_unreachable();
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

char *itoa(int num, char *str, int base) {
    int i = 0;
    bool isNegative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        num = num/base;
    }

    if (isNegative)
        str[i++] = '-';

    str[i] = '\0';
    _reverse(str, i);
    return str;
}
