#include <stdbool.h>
#include "libk.h"
#include "mm.h"

struct mmap_entry {
	uintptr_t base;
	size_t size;
	uint64_t type;
	struct mmap_entry *next;
};

struct mmap_entry *mmap_list;
struct mmap_entry *mmap_head;

static struct multiboot_tag *get_next_tag(struct multiboot_tag *tag) {
	size_t size = tag->size;
	if (tag->type == MULTIBOOT_TAG_TYPE_END && size == 8)
		return NULL;

	uintptr_t aligned = (((uintptr_t)tag + size + 7) & -8);
	return (struct multiboot_tag *)aligned;
}

static void mboot_mmap_parse(struct multiboot_tag_mmap *mmap) {
	kassert(mmap->entry_version == 0);
	for (size_t i = 0; i < (mmap->size / mmap->entry_size); i++) {
		mmap_head->base = mmap->entries[i].addr;
		mmap_head->size = mmap->entries[i].len;
		mmap_head->type = mmap->entries[i].type;
		mmap_head->next = boot_heap_malloc(sizeof(struct mmap_entry));
		mmap_head = mmap_head->next;
	}
}

void pmm_mmap_parse(struct multiboot_info *mboot) {
	kprintf("Multiboot info address: %p\n", mboot);

	mmap_list = boot_heap_malloc(sizeof(struct mmap_entry));
	mmap_head = mmap_list;

	struct multiboot_tag *current_tag = mboot->tags;
	kassert(current_tag->type != 0);

	do {
		switch (current_tag->type) {
			case MULTIBOOT_TAG_TYPE_MMAP:
				mboot_mmap_parse((struct multiboot_tag_mmap *)current_tag);
				break;
		}
	} while ((current_tag = get_next_tag(current_tag)) != NULL);

	struct mmap_entry *temp;
	for (mmap_head = mmap_list; mmap_head != NULL; mmap_head = temp) {
		if (mmap_head->type == MULTIBOOT_MEMORY_AVAILABLE) {
			kprintf("Memory map entry:\n");
			kprintf("\tBase address: %p\n", (void*)mmap_head->base);
			kprintf("\tEnd address: %p\n", (void*)(mmap_head->base + mmap_head->size));
			kprintf("\tLength: %zu\n", mmap_head->size);
		}
		temp = mmap_head->next;
		boot_heap_free(mmap_head);
	}
}
