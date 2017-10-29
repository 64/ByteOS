#pragma once

#include <stddef.h>

int memcmp(const void *, const void *, size_t);
void* memcpy(void * restrict, const void * restrict, size_t);
void* memmove(void *, const void *, size_t);
void* memset(void *, int, size_t);
size_t strlen(const char *);

__attribute__((format (printf, 1, 2))) int kprintf(const char *, ...);

__attribute__((noreturn)) void abort(void);

#define panic(...) do { \
		kprintf( \
			"\n\x1B[0m--------------------------------------------------------------------------------\x1B[0m" \
			"\x1B[1;41;37mpanic at %s:%s:%u\x1B[0m\n", \
			__FILE__, __func__, __LINE__ \
		); \
		kprintf(__VA_ARGS__); \
		abort(); \
	} while(0)

#define kassert(condition) do { \
		if (!(condition)) \
			panic("kassert condition failed: %s\n", #condition); \
	} while(0)

#ifdef NDEBUG
	#define kassert_dbg(x)
#else
	#define kassert_dbg(x) kassert(x)
#endif
