#pragma once

#include <stddef.h>

// Compiler magic to find the original struct (taken from linux kernel)
#define container_of(ptr, type, member) ({                      \
        const typeof(((type *)0)->member) *__mptr = (ptr);    \
        type *__t = (type *)((char *)__mptr - offsetof(type, member)); \
	__mptr == NULL ? NULL : __t; })
