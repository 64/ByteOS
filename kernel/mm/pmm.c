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
		if (mmap->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE)
			boot_heap_register_node(mmap->entries[i].addr, mmap->entries[i].len);
	}
}

void pmm_mmap_parse(struct multiboot_info *mboot) {
	struct multiboot_tag *current_tag = mboot->tags;
	kassert(current_tag->type != 0);
	boot_heap_init();

	do {
		switch (current_tag->type) {
			case MULTIBOOT_TAG_TYPE_MMAP:
				mboot_mmap_parse((struct multiboot_tag_mmap *)current_tag);
				break;
		}
	} while ((current_tag = get_next_tag(current_tag)) != NULL);
}
