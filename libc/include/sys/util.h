#pragma once

#ifndef UNUSED
#	if defined(__GNUC__)
#		define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#	elif defined(__LCLINT__)
#		define UNUSED(x) /*@unused@*/ x
#	else
#		error "This compiler is not currently supported."
#	endif
#endif
