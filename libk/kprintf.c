#include <stdarg.h>
#include <stdbool.h>

#include "libk.h"
#include "drivers/vga_tmode.h"
#include "drivers/serial.h"

#define to_hex_char(x) ((x) < 10 ? (x) + '0' : (x) - 10 + 'A')
#define ATOI_BUFLEN 256

static inline void kprintf_write_char(char c)
{
	serial_write_com(1, c);
	vga_tmode_putchar(c);
}

static inline void kprintf_write_str(char *s)
{
	while (*s)
		kprintf_write_char(*s++);
}

static int atoi_print(uint64_t num, bool sign)
{
	char buf[ATOI_BUFLEN], *p_buf = buf + sizeof buf - 2;
	int nwritten = 0;
	buf[ATOI_BUFLEN - 1] = '\0';

	if (num == 0) {
		kprintf_write_char('0');
		return 1;
	} else if (sign) {
		int64_t n = (int64_t)num;
		if (n < 0) {
			kprintf_write_char('-');
			nwritten++;
			n = -n;
		}
		while (n != 0) {
			*p_buf-- = (n % 10) + '0';
			n /= 10;
			nwritten++;
		}
	} else {
		uint64_t n = num;
		while (n != 0) {
			*p_buf-- = (n % 10) + '0';
			n /= 10;
			nwritten++;
		}
	}
	kprintf_write_str(p_buf + 1);
	return nwritten;
}

int kprintf(const char *fmt, ...)
{
	va_list params;
	size_t nwritten = 0;
	const char *pfmt = fmt;

	va_start(params, fmt);

	while (*pfmt) {
		if (*pfmt == '%') {
			switch (pfmt[1]) {
				case '%':
					kprintf_write_char('%');
					nwritten++;
					break;
				case 'c':
					kprintf_write_char((char)va_arg(params, int));
					nwritten++;
					break;
				case 's': {
					const char *s = va_arg(params, const char *);
					kprintf_write_str((char *)s);
					nwritten += strlen(s);
					break;
				}
				case 'p': {
					char buf[17], *p_buf = buf + 15;
					buf[16] = '\0';
					uintptr_t ptr = (uintptr_t)va_arg(params, void *);
					while (p_buf >= buf) {
						*p_buf-- = to_hex_char(ptr & 0xF);
						ptr >>= 4;
					}
					kprintf_write_char('0');
					kprintf_write_char('x');
					kprintf_write_str(buf);
					nwritten += 18;
					break;
				}
				case 'l':
					if (pfmt[2] == 'l')
						pfmt++;
					__attribute__((fallthrough));
				case 'z':
					if (pfmt[2] == 'd' || pfmt[2] == 'i')
						nwritten += atoi_print(va_arg(params, long), true);
					else if (pfmt[2] == 'u')
						nwritten += atoi_print(va_arg(params, unsigned long), false);
					pfmt++;
					break;
				case 'i':
				case 'd':
					nwritten += atoi_print(va_arg(params, int), true);
					break;
				case 'u':
					nwritten += atoi_print(va_arg(params, unsigned int), false);
					break;
				default:
					panic("kprintf: unknown format string character '%c'", pfmt[1]);
					break;
			}
			pfmt++;
			nwritten++;
		} else
			kprintf_write_char(*pfmt);
		pfmt++;
	}

	va_end(params);
	return nwritten;
}
