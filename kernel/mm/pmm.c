#include <stdint.h>
#include "mm.h"
#include "libk.h"
#include "util.h"

struct zone *zone_list;

// Buddy allocator reserves some pages as overhead for the region
static inline uint64_t calc_available_pages(struct mmap_region *available)
{
	// TODO: Check we aren't losing any bytes here
	uintptr_t effective_start = ALIGNUP(available->base, PAGE_SIZE);
	size_t effective_len = ALIGNDOWN(available->len, PAGE_SIZE);
	size_t overhead = sizeof(struct zone) + sizeof(struct page) * (effective_len / PAGE_SIZE);
	uintptr_t final_available_start = MAX(effective_start, available->base + overhead);
	size_t final_available_len = ALIGNDOWN(effective_len - (final_available_start - effective_start), PAGE_SIZE);
	return final_available_len / PAGE_SIZE;
}

static struct zone *init_zone(struct mmap_region *rg)
{
	struct zone *zone = phys_to_virt(rg->base);
	size_t avail_pages = calc_available_pages(rg);
	if (avail_pages == 0) // Not enough memory to be useful, so ignore it
		return NULL;

	zone->len = avail_pages * PAGE_SIZE;
	zone->pa_start = ALIGNUP(rg->base + sizeof(struct zone) + avail_pages * (sizeof(struct page)), PAGE_SIZE);
	memset(zone->free_lists, 0, sizeof(zone->free_lists));
	memset(zone->pgdata, 0, avail_pages * sizeof(struct page)); // Zero all struct pages

	// Populate the free_lists
	size_t pages_inserted = 0;
	while (pages_inserted < avail_pages) {
		size_t remaining = avail_pages - pages_inserted;
		// TODO: Make this not give me eye cancer
		size_t highest_order = MAX_ORDER - (__builtin_clz(MIN(remaining, 1LU << (MAX_ORDER - 1))) - __builtin_clz((1 << MAX_ORDER)));
		kassert(highest_order < MAX_ORDER);
		struct page *inserted = &zone->pgdata[pages_inserted];
		inserted->order = highest_order;
		slist_set_next(inserted, list, zone->free_lists[highest_order]);
		zone->free_lists[highest_order] = inserted;
		pages_inserted += (1 << highest_order);
	}
	kassert(pages_inserted == avail_pages);

	// TODO: Remove this
	// Write a value at the last page to ensure we don't actually segfault
	volatile uint64_t *dummy = (void *)((uintptr_t)zone + avail_pages * PAGE_SIZE + (PAGE_SIZE - sizeof(uint64_t)));
	*dummy = 0x0123456789ABCDEF;
	kassert(*dummy == 0x0123456789ABCDEF);
	return zone;
}


void pmm_init(struct mmap *mmap)
{
	struct zone *tail = NULL;
	for (size_t i = 0; i < mmap->available.count; i++) {
		struct zone *tmp = init_zone(&mmap->available.regions[i]);
		if (tail == NULL)
			zone_list = tmp;
		else
			slist_set_next(tail, list, tmp);
		tail = tmp;
	}

	slist_foreach(zone, list, zone_list) {
		kprintf("Zone: %p virtual, %zu pages\n", (void *)zone, zone->len / PAGE_SIZE);
	}
}

static struct page *page_buddy(struct zone *zone, struct page *page, unsigned int order)
{
	size_t pgdata_index = page - zone->pgdata;	
	size_t buddy_index = pgdata_index ^ (1 << order);
	return &zone->pgdata[buddy_index];
}

static struct page *zone_alloc_order(struct zone *zone, unsigned int order, unsigned int alloc_flags)
{
	(void)alloc_flags;
	if (zone->free_lists[order] == NULL) {
		// Need to split up zones
		unsigned int free_order;
		// Scan to the next free order
		for (free_order = order + 1; free_order < MAX_ORDER; free_order++) {
			if (zone->free_lists[free_order] != NULL)
				break;
		}
		// If nothing was free, then we're screwed
		if (free_order == MAX_ORDER)
			return NULL;
		// Keep splitting downwards until there is room in free_lists[order]
		for (; free_order > order; free_order--) {
			kprintf("Splitting page of order %u\n", free_order);
			struct page *removed = zone->free_lists[free_order];
			struct page *buddy = page_buddy(zone, removed, free_order - 1);
			removed->order = buddy->order = free_order - 1;
			zone->free_lists[free_order] = slist_get_next(zone->free_lists[free_order], list);
			slist_set_next(removed, list, buddy);
			slist_set_next(buddy, list, zone->free_lists[free_order - 1]);
			zone->free_lists[free_order - 1] = removed;
		}
	}
	struct page *head = zone->free_lists[order];
	kassert(head->order == order);
	kassert(head != NULL);
	zone->free_lists[order] = slist_get_next(head, list);
	slist_set_next(head, list, (struct page *)NULL); // Not strictly necessary
	return head;
}

struct page *pmm_alloc_order(unsigned int order, unsigned int alloc_flags)
{
	slist_foreach(zone, list, zone_list) {
		// TODO: Early check zone with page count before alloc
		struct page *rv = zone_alloc_order(zone, order, alloc_flags);
		if (rv != NULL)
			return rv;
	}
	return NULL;
}


