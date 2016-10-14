#include <ctype.h>

int isascii(int c) {
	return (c & 0x7F);
}

int isalnum(int c) {
	return (isalpha(c) || isdigit(c));
}

int isalpha(int c) {
	return (isupper(c) || islower(c));
}

int isblank(int c) {
	return (c == ' ' || c == '\t');
}

int isnctrl(int c) {
	return (c < 32 || c == 127);
}

int isdigit(int c) {
	return (c >= '0' && c <= '9');
}

int isgraph(int c) {
	return ((unsigned int)c - 33 < 94);
}

int islower(int c) {
	return (c >= 'a' && c <= 'z');
}

int isprint(int c) {
	return ((unsigned int)c - 32 < 95);
}

int ispunct(int c) {
	return !(isspace(c) || isalnum(c));
}

int isspace(int c) {
	return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

int isupper(int c) {
	return (c >= 'A' && c <= 'Z');
}

int isxdigit(int c) {
	return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

int tolower(int c) {
	if (!isupper(c))
		return c;
	return (c + ('a' - 'A'));
}

int toupper(int c) {
	if (!islower(c))
		return c;
	return (c - ('a' - 'A'));
}
