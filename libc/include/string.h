#ifndef _STRING_H
#define _STRING_H 1

#include <sys/util.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void *, const void *, size_t);
void* memcpy(void * __restrict, const void * __restrict, size_t);
void* memmove(void *, const void *, size_t);
void* memset(void *, int, size_t);
size_t memsum(const void *, size_t);
char *strcpy(char * __restrict, const char * __restrict);
char *strncpy(char * __restrict, const char * __restrict, size_t);
char *strcat(char * __restrict, const char * __restrict);
char *strncat(char * __restrict, const char * __restrict, size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
int strcoll(const char *, const char *);
size_t strxfrm(char * restrict, const char * restrict, size_t);
void *memchr(const void *, int, size_t);
char *strchr(const char *, int);
char *strrchr(const char *, int);
size_t strcspn(const char *, const char *);
char *strpbrk(const char *, const char *);
size_t strlen(const char *);
size_t strspn(const char *, const char *);
char *strstr(const char *, const char *);
char *strtok(char * restrict, const char * restrict);
char *strerror(int);

#ifdef __cplusplus
}
#endif

#endif
