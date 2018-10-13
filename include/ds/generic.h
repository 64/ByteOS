#pragma once

// Compiler magic to find the original struct (taken from linux kernel)
#define container_of(ptr, type, member) ({                      \
        const typeof(((type *)0)->member) *__mptr = (ptr);    \
        (type *)((char *)__mptr - offsetof(type, member)); })

