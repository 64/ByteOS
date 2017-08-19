#include "libk.h"

int memcmp(const void *s1, const void *s2, size_t n) {
	const unsigned char *a = s1;
	const unsigned char *b = s2;
	for (size_t i = 0; i < n; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

void* memcpy(void * restrict s1, const void* restrict s2, size_t n) {
	unsigned char *dst = s1;
	const unsigned char *src = s2;
	for (size_t i = 0; i < n; i++)
		dst[i] = src[i];
	return s1;
}

void* memmove(void *s1, const void *s2, size_t n) {
	unsigned char *dst = s1;
	const unsigned char *src = s2;
	if (s1 < s2) {
		for (size_t i = 0; i < n; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = n; i > 0; i--)
			dst[i - 1] = src[i - 1];
	}
	return s1;
}

void* memset(void *s, int c, size_t n) {
	unsigned char *p = s, *end = p + n;
	for (; p != end; p++) {
		*p = (unsigned char)c;
	}
	return s;
}

size_t strlen(const char *s) {
	size_t len = 0;
	while (*s++)
		len++;
	return len;
}
