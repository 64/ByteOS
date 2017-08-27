#pragma once

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define ALIGNUP(val, align) (((val) + (align - 1)) & -(align))
