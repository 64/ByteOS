#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/util.h>
#include <stdarg.h>
#include <stddef.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char * restrict format, ...);
int sprintf(char * restrict s, const char * restrict format, ...);
int snprintf(char * restrict s, size_t n, const char * restrict format, ...);
int vprintf(const char * restrict format, va_list arg);
int vsnprintf(char * restrict s, size_t n, const char * restrict format, va_list arg);
int vsprintf(char * restrict s, const char * restrict format, va_list arg);

int putchar(int);
int puts(const char*);

#ifdef __cplusplus
}
#endif

#endif
