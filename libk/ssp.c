#include <stdint.h>
#include "libk.h"

uintptr_t __stack_chk_guard = 0xc9073a36f93e7732;

__attribute__((noreturn)) void __stack_chk_fail(void)
{
	// TODO: Call a special exception handler that swaps stacks?
	panic("Stack smashing detected");	
}
