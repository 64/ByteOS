#pragma once

#ifdef __GNUC__
#       define COMPILER_ATTR_NORETURN __attribute__((noreturn))
#       define COMPILER_ATTR_PACKED __attribute__((packed))
#	define COMPILER_ATTR_ALIGN(al) __attribute__((align(al)))
#       define COMPILER_ATTR_CONSTRUCTOR __attribute__((constructor))
#       define COMPILER_ATTR_USED __attribute__((used))
#       define COMPILER_BUILTIN_UNREACHABLE __builtin_unreachable()
#else
#       error "This compiler is not supported."
#endif
