#pragma once

#include <stdint.h>

#define RETURNS_ERROR err_t __attribute__((warn_unused_result))

typedef int64_t err_t;

#include "gen/err_gen.h"
