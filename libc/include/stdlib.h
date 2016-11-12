#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/util.h>
#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif

COMPILER_ATTR_NORETURN
void abort(void);

long int strtol(const char * restrict nptr, char ** restrict endptr, int base);

char *itoa(uint32_t, char *, uint32_t);

#ifdef __cplusplus
	}
#endif

#endif
