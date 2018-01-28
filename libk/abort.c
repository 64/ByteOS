#include "libk.h"

__attribute__((noreturn)) void abort(void)
{
	asm volatile (
		"cli\n"
		".stop: hlt\n"
		"jmp .stop\n"
		:
		:
		:
	);
	__builtin_unreachable();
}
