#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern char error_header[];
extern char info_header[];

uintptr_t __stack_chk_guard = 0x0;

COMPILER_ATTR_CONSTRUCTOR void ssp_init(void) {
    // TODO: Randomize this
    __stack_chk_guard = 0xE2DEE396;
}

COMPILER_ATTR_NORETURN void __stack_chk_fail(void) {
    printf("%s Stack smashing detected\n", error_header);
    abort();
}
