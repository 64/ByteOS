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
	while ((*s1++ = *s2++))
		;
	return rv;
}

char *strncpy(char * restrict s1, const char * restrict s2, size_t n) {
	char *rv = s1;
	while ((n > 0) && (*s1++ = *s2++))
		n--;
	while (n-- > 1)
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
	while(n && (*s1++ = *s2++))
		n--;
	if (n == 0)
		*s1 = '\0';
	return rv;
}

int strcmp(const char * s1, const char * s2) {
	while (*s1 && (*s1 == *s2))
		s1++, s2++;
	return *((unsigned char * restrict)s1) - *((unsigned char * restrict)s2);
}

int strncmp(const char * s1, const char * s2, size_t n) {
	while (*s1 && n && (*s1 == *s2)) {
		s1++, s2++;
		n--;
	}
	if (n == 0)
		return 0;
	return *((const unsigned char *)s1) - *((const unsigned char *)s2);
}

int strcoll(const char *s1, const char *s2) {
	return strcmp(s1, s2); // TODO: This needs to be dependend on locale
}

size_t strxfrm(char * restrict s1, const char * restrict s2, size_t n) {
	size_t len = strlen(s2);
	if (len > n)
		while (n-- && (*s1++ = *s2++))
			;
	return len;
}

void *memchr(const void *s, int c, size_t n) {
	const unsigned char *p = s;
	while (n--) {
		if (*p == (unsigned char)c)
			return (void*)p;
		p++;
	}
	return NULL;
}

char *strchr(const char *s, int c) {
	const char *p = s;
	while (*p) {
		if (*p == (char)c)
			return (char *)p;
		p++;
	}
	return NULL;
}

char *strrchr(const char *s, int c) {
	size_t len = strlen(s);
	const char *end = s + len; // Includes null byte
	do {
		if (*end == c)
			return (char *)end;
		end--;
	} while (end != s);
	return NULL;
}

size_t strcspn(const char *s1, const char *s2) {
	size_t len = 0;
	while (*s1 && strchr(s2, *s1++) == NULL)
		len++;
	return len;
}

char *strpbrk(const char *s1, const char *s2) {
	const char *p1 = s1;
	const char *p2 = s2;
	while (*p1) {
		p2 = s2;
		while (*p2) {
			if (*p2++ == *p1)
				return (char *)p1;
		}
		p1++;
	}
	return NULL;
}

size_t strspn(const char *s1, const char *s2) {
	size_t len = 0;
	while (*s1 && strchr(s2, *s1++) != NULL)
		len++;
	return len;
}

char *strstr(const char *s1, const char *s2) {
	const char *p1 = s1;
	const char *p2 = s2;
	while (*s1) {
		p2 = s2;
		while (*p2 && (*p2 == *p1))
			p1++, p2++;

		if (*p2 == 0)
			return (char *)s1;

		s1++;
		p1 = s1;
	}
	return NULL;
}

char *strtok(char * restrict s1, const char * restrict s2) {
	static char *temp = NULL;
	const char *p = s2;

	if (s1 == NULL) {
		if (temp == NULL)
			return NULL;
		s1 = temp;
	} else
		temp = s1;

	while ( *p && *s1 ) {
		if ( *s1 == *p ) {
			s1++;
			p = s2;
			continue;
		}
		p++;
	}

	if (*s1 == 0)
		return (temp = NULL);

	temp = s1;
	while (*temp) {
		p = s2;
		while (*p)
			if (*temp == *p++) {
				*temp++ = '\0';
				return s1;
			}
		temp++;
	}

	temp = NULL;
	return s1;
}

char *strerror(int errnum) {
	return (char *)"Unknown Error (TODO)";
}
