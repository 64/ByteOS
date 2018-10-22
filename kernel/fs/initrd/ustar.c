#include "fs/initrd.h"
#include "util.h"

#define USTAR_BLOCK 512

// https://wiki.osdev.org/USTAR
uint32_t ustar_oct_to_bin(const char *str, size_t len)
{
	uint32_t n = 0;
	const uint8_t *c = (const uint8_t *)str;
	while (len-- > 0) {
		n *= 8;
		n += *c - '0';
		c++;
	}
	return n;
}

struct ustar_header *ustar_nth_from_entry(struct ustar_header *root_hdr, uint32_t n)
{
	struct ustar_header *cur = root_hdr;

	for (uint32_t i = 0; i < n; i++) {
		if (cur->filename[0] == '\0')
			return NULL; // EOF

		uint32_t size = ustar_oct_to_bin(cur->size, sizeof cur->size - 1);	

		// Get next header
		uintptr_t next = ALIGNUP((uintptr_t)cur + size, USTAR_BLOCK);
		cur = (struct ustar_header *)next;	
	}

	return cur;
}

virtaddr_t ustar_file_start(struct ustar_header *hdr)
{
	return (virtaddr_t)((uintptr_t)hdr + USTAR_BLOCK);
}
