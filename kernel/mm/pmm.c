#include <stdint.h>
#include "mm.h"
#include "libk.h"
#include "util.h"

struct buddy_allocator {
	size_t overhead; // Size of this struct in bytes
	uint64_t nr_owned, nr_free; // Number of pages owned / free respecively
	struct page *free_lists[MAX_ORDER];
	struct page page_data[]; // All the pages owned by this allocator
};

// Buddy allocator reserves some pages as overhead for the region
static inline uint64_t calc_available_pages(struct mmap_region *available)
{
	// TODO: Prove that this is correct, especially given integer division etc
	// overhead = ALIGN(sizeof(struct buddy_allocator) + sizeof(struct page) * (available_bytes / PAGE_SIZE))
	// available_bytes = (total_memory - overhead)
	// o = ba + ssp * (a / pgsize)
	// a = len - o
	// a = len - (ba + ssp * (a / pgsize))
	// a = len - ba - ssp * (a / pgsize)
	// a + ssp * (a / pgsize) = len - ba
	// a * pgsize + ssp * a = pgsize * (len - ba)
	// a * (pgsize + ssp) = pgsize * (len - ba)
	// a = pgsize * (len - ba) / (pgsize + ssp)
	return (available->len - sizeof(struct buddy_allocator)) / (PAGE_SIZE + sizeof(struct page));
}

static void init_region(struct mmap_region *rg)
{
	const uint64_t pg_avail = calc_available_pages(rg);
	const size_t overhead = sizeof(struct buddy_allocator) + sizeof(struct page) * pg_avail;
	// Skip this region as there is not enough usable memory
	// TODO: Don't waste this
	if (pg_avail == 0)
		return;

	// Instantiate the buddy allocator
	struct buddy_allocator *ba = (struct buddy_allocator *)phys_to_virt(rg->base);
	ba->overhead = overhead;
	ba->nr_owned = ba->nr_free = pg_avail;
	memset(ba->free_lists, 0, sizeof(ba->free_lists));

	// Instantiate the struct pages
	const uintptr_t avail_start = ALIGNUP((uintptr_t)ba + ba->overhead, PAGE_SIZE);
	for (size_t i = 0; i < ba->nr_owned; i++) {
		ba->page_data[i].virt = (virtaddr_t)(avail_start + i * PAGE_SIZE);
		ba->page_data[i].nr_mapped = 0;
		ba->page_data[i].order = 0;
		slist_set_next(&ba->page_data[i], list, (struct page *)NULL);

		// TODO: Remove this
		// Write a value at each location to ensure we don't actually segfault
		volatile uint64_t *dummy = (uint64_t *)ba->page_data[i].virt + (PAGE_SIZE - sizeof(uint64_t));
		*dummy = 0x0123456789ABCDEF;
		kassert(*dummy == 0x0123456789ABCDEF);
		if (dummy > (uint64_t *)phys_to_virt(0x1000000)) {
			kprintf("Dummy is %p at %zu/%zu", (void *)dummy, i, ba->nr_owned);
			panic("Out of bounds write");
		}
	}

	kprintf("Buddy allocator at %p with %zu free pages\n", (void *)ba, ba->nr_owned);
	kprintf("Available memory starts at %p\n", (void *)avail_start);
}


void pmm_init(struct mmap_type *available)
{
	for (size_t i = 0; i < available->count; i++) {
		init_region(&available->regions[i]);
	}
}

struct page *pmm_region_alloc_page(struct mmap_region *rg, unsigned int alloc_flags)
{
	(void)rg;
	(void)alloc_flags;
	return NULL;
}


