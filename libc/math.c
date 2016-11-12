#include <math.h>

// TODO: Add errno stuff
double pow(double x, double y) {
	return __builtin_pow(x, y);
}

double floor(double x) {
	return __builtin_floor(x);
}

double modf(double value, double *iptr) {
	return __builtin_modf(value, iptr);
}

double fmod(double x, double y) {
	return __builtin_fmod(x, y);
}

double fabs(double x) {
	if (x < 0)
		return -x;
	return x;
}

double log10(double x) {
	return __builtin_log10(x);
}
