#include "snow.h"

describe(string, {

	subdesc(memcpy, {
		it("copies 10 bytes", {
			char dst[10] = { 0 };
			char src[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			__libk_memcpy(dst, src, 10);
			asserteq_buf(dst, src, 10);
		});
	});

	subdesc(memset, {
		it("sets 10 bytes", {
			char dst[10] = { 0 };
			char expected[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			__libk_memset(dst, 1, 10);
			asserteq_buf(dst, expected, 10);
		});
	});

	subdesc(memmove, {
		it("moves 10 non-overlapping bytes", {
			char dst[10] = { 0 };
			char src[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			__libk_memcpy(dst, src, 10);
			asserteq_buf(dst, src, 10);
		});

		it("moves 10 low-overlapping bytes", {
			char buf[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			char expected[10] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
			__libk_memmove(buf, buf + 4, 10);
			asserteq_buf(buf, expected, 10);
		}); 
	
		it("moves 10 high-overlapping bytes", {
			char buf[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			char expected[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			__libk_memmove(buf + 4, buf, 10);
			asserteq_buf(buf + 4, expected, 10);
		}); 
	});

	subdesc(memcmp, {
		it("compares 10 equal bytes", {
			char a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			char b[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			asserteq(__libk_memcmp(a, b, 10), 0);
		});

		it("compares 10 non-equal bytes", {
			char a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			char b[10] = { 1, 2, 4, 4, 5, 6, 7, 8, 9, 10 };
			assertneq(__libk_memcmp(a, b, 10), 0);
		});
	});

	subdesc(strlen, {
		it("returns 10 for a string of length 10", {
			char *tst = "0123456789";
			asserteq(__libk_strlen(tst), 10);
		});

		it("returns 0 on a null byte", {
			char null = '\0';
			asserteq(__libk_strlen(&null), 0);
		});
	});
});
