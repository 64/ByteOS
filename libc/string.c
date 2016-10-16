#include <string.h>

int memcmp(const void* aptr, const void* bptr, size_t size) {
	const unsigned char* a = (const unsigned char*) aptr;
	const unsigned char* b = (const unsigned char*) bptr;
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

void* memmove(void* dstptr, const void* srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

void* memset(void* bufptr, int value, size_t size) {
	unsigned char* buf = (unsigned char*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (unsigned char) value;
	return bufptr;
}

// My own function, helps with checksums
size_t memsum(void *ptr, size_t size) {
	size_t total = 0;
	unsigned char *p = ptr;
	while(size--)
		total += *p++;
	return total;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

// TODO: Finish these
char *strcpy(char * restrict s1, const char * restrict s2) {
	char *rv = s1;
	while (*s2)
		*s1++ = *s2++;
	*s1 = '\0';
	return rv;
}

char *strncpy(char * restrict s1, const char * restrict s2, size_t n) {
	char *rv = s1;
	while (*s2 && n-- != 0)
		*s1++ = *s2++;
	while (n-- != 0)
		*s1++ = '\0';
	return rv;
}

char *strcat(char * restrict s1, const char * restrict s2) {
	// Scan until null byte
	char *rv = s1;
	while (*s1)
		s1++;
	strcpy(s1, s2);
	return rv;
}

char *strncat(char * restrict s1, const char * restrict s2, size_t n) {
	char *rv = s1;
	while (*s1)
		s1++;
	strncpy(s1, s2, n);
	return rv;
}

char *strcmp(char * restrict s1, char * restrict s2) {
	return NULL;
}

char *strncmp(char * restrict s1, char * restrict s2, size_t n) {
	return NULL;
}

int strcoll(const char *s1, const char *s2) {
	return 0;
}

size_t strxfrm(char * restrict s1, const char * restrict s2, size_t n) {
	return 0;
}

void *memchr(const void *s, int c, size_t n) {
	return NULL;
}

char *strchr(const char *s, int c) {
	return NULL;
}

char *strrchr(const char *s, int c) {
	return NULL;
}

size_t strcspn(const char *s1, const char *s2) {
	return 0;
}

char *strpbrk(const char *s1, const char *s2) {
	return NULL;
}

size_t strspn(const char *s1, const char *s2) {
	return 0;
}

char *strstr(const char *s1, const char *s2) {
	return NULL;
}

char *strtok(char * restrict s1, const char * restrict s2) {
	return NULL;
}

char *strerror(int errnum) {
	return NULL;
}
