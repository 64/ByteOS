#include <stdbool.h>
#include "util.h"
#include "libk.h"
#include "mm.h"

static struct multiboot_tag *get_next_tag(struct multiboot_tag *tag) {
	size_t size = tag->size;
	if (tag->type == MULTIBOOT_TAG_TYPE_END && size == 8)
		return NULL;

	uintptr_t aligned = ALIGNUP((uintptr_t)tag + size, 8);
	return (struct multiboot_tag *)aligned;
}

static void mboot_mmap_parse(struct multiboot_tag_mmap *mmap) {
	kassert(mmap->entry_version == 0);
	for (size_t i = 0; i < (mmap->size / mmap->entry_size); i++) {
		if (mmap->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
			physaddr_t start_addr = mmap->entries[i].addr;
			size_t len = mmap->entries[i].len;
			if (!(start_addr + len > KERNEL_PHYS_MAP_END))
				continue;
			else if (start_addr < KERNEL_PHYS_MAP_END) {
				len -= KERNEL_PHYS_MAP_END - start_addr;
				start_addr = KERNEL_PHYS_MAP_END;
			}
			boot_heap_free_pages_phys(start_addr, len / PAGE_SIZE);
		}
	}
}

void pmm_mmap_parse(struct multiboot_info *mboot) {
	boot_heap_init();

	struct multiboot_tag *current_tag = mboot->tags;
	kassert(current_tag->type != 0);

	do {
		switch (current_tag->type) {
			case MULTIBOOT_TAG_TYPE_MMAP:
				mboot_mmap_parse((struct multiboot_tag_mmap *)current_tag);
				break;
			default:
				break;
		}
	} while ((current_tag = get_next_tag(current_tag)) != NULL);
}
