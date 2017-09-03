#pragma once

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define ALIGNUP(val, align) (((val) + (align - 1)) & -(align))
#define ISALIGN_POW2(val, align) (((val) & ((align) - 1)) & ((val) - 1) == 0)
#define UNUSED(x) __attribute__((unused)) x
