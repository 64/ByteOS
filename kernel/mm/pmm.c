#include <stdint.h>
#include "mm.h"
#include "libk.h"
#include "util.h"

struct zone *zone_list;
spinlock_t zone_list_lock;

struct page *const page_data = (struct page *)KERNEL_PAGE_DATA;

// Buddy allocator reserves some pages as overhead for the region
static inline uint64_t calc_available_pages(struct mmap_region *available)
{
	// TODO: Check we aren't losing any bytes here
	physaddr_t effective_start = ALIGNUP(available->base + sizeof(struct zone), PAGE_SIZE);
	size_t effective_len = available->len - (effective_start - (available->base + sizeof(struct zone)));
	kassert_dbg(effective_len <= available->len);
	return effective_len / PAGE_SIZE;
}

static void reserve_page_data_range(physaddr_t start, size_t num_pages)
{
	size_t pfn_start = (start / PAGE_SIZE);
	size_t pfn_end = pfn_start + num_pages;
	for (size_t pfn = pfn_start; pfn < pfn_end; pfn++) {
		virtaddr_t array_addr = page_data + pfn;
		// Probably won't happen, but in case this area isn't mapped, map it
		if (!vmm_has_flags(&kernel_mmu, array_addr, PAGE_PRESENT)) {
			// Need to alloc + map
			struct mmap_region rg = mmap_alloc_low(PAGE_SIZE, MMAP_ALLOC_PA);
			kprintf("Allocated struct pages for pfn %zu\n", pfn);
			kassert(rg.len == PAGE_SIZE);
			vmm_map_page(&kernel_mmu, (physaddr_t)rg.base, array_addr, PAGE_WRITABLE | PAGE_GLOBAL);
		}
	}
	memset(page_data + pfn_start, 0, (pfn_end - pfn_start) * sizeof(struct page));
}

static void reserve_page_data(struct mmap_type *available)
{
	for (size_t i = 0; i < available->count; i++) {
		struct mmap_region *rg = &available->regions[i];
		reserve_page_data_range(ALIGNUP(rg->base, PAGE_SIZE), calc_available_pages(rg));
	}
}

static size_t get_max_order(uintptr_t start, uintptr_t end)
{
	size_t start_pfn = start / PAGE_SIZE, end_pfn = end / PAGE_SIZE, rv;
	// TODO: Calculate this analytically	
	for (rv = MAX_ORDER - 1; rv > 0; rv--) {
		if (start_pfn + (1 << rv) > end_pfn)
			continue;
		if (ISALIGN_POW2(start_pfn, (1 << rv)))
			break;
	}
	kassert_dbg(rv < MAX_ORDER);
	return rv;
}

static struct zone *init_zone(struct mmap_region *rg)
{
	struct zone *zone = phys_to_virt(rg->base);
	size_t avail_pages = calc_available_pages(rg);
	if (avail_pages == 0) // Not enough memory to be useful, so ignore it
		return NULL;

	zone->len = avail_pages * PAGE_SIZE;
	zone->pa_start = ALIGNUP(rg->base + sizeof(struct zone), PAGE_SIZE);
	memset(zone->free_lists, 0, sizeof(zone->free_lists));
	slist_set_next(zone, list, (struct zone *)NULL);

	// Populate the free_lists
	size_t pages_inserted = 0;
	while (pages_inserted < avail_pages) {
		size_t highest_order = get_max_order(zone->pa_start + pages_inserted * PAGE_SIZE, zone->pa_start + avail_pages * PAGE_SIZE);
		struct page *inserted = virt_to_page(phys_to_virt(zone->pa_start + PAGE_SIZE * pages_inserted));
		inserted->order = highest_order;
		dlist_set_next(inserted, list, zone->free_lists[highest_order]);
		zone->free_lists[highest_order] = inserted;
		kassert_dbg(page_to_phys(inserted) + PAGE_SIZE * (1 << highest_order) <= zone->pa_start + zone->len);
		pages_inserted += (1 << highest_order);
	}
	kassert(pages_inserted == avail_pages);

#ifdef DEBUG
	// Write a value at the last page to ensure we don't actually segfault
	volatile uint64_t *dummy = (void *)((uintptr_t)zone + avail_pages * PAGE_SIZE + (PAGE_SIZE - sizeof(uint64_t)));
	*dummy = 0x0123456789ABCDEF;
	kassert(*dummy == 0x0123456789ABCDEF);
#endif
	return zone;
}


void pmm_init(struct mmap *mmap)
{
	struct zone *tail = NULL;
	spin_init(&zone_list_lock);

	// Copy into a new temporary array, since allocating might edit the array underneath us, so we need a separate copy
	struct mmap_region temp_rgs[MMAP_MAX_REGIONS];
	struct mmap_type temp_avail = {
		.count = mmap->available.count,
		.regions = temp_rgs
	};
	for (size_t i = 0; i < mmap->available.count; i++)
		memcpy(temp_rgs + i, &mmap->available.regions[i], sizeof(struct mmap_region));

	reserve_page_data(&temp_avail);
	for (size_t i = 0; i < temp_avail.count; i++) {
		struct zone *tmp = init_zone(&temp_rgs[i]);
		if (tmp == NULL) // Not enough space to be usable
			continue;
		if (tail == NULL) {
			zone_list = tmp;
		} else
			slist_set_next(tail, list, tmp);
		tail = tmp;
	}

	slist_foreach(zone, list, zone_list) {
		klog("pmm", "Zone: %p - %p physical, %zu pages\n", (void *)virt_to_phys(zone),
			(void *)(virt_to_phys(zone) + zone->len), zone->len / PAGE_SIZE);
	}
}

static struct page *page_buddy(struct page *page, unsigned int order)
{
	size_t pgdata_index = page - page_data;
	size_t buddy_index = pgdata_index ^ (1 << order);
	return &page_data[buddy_index];
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
			struct page *removed = zone->free_lists[free_order];
			struct page *buddy = page_buddy(removed, free_order - 1);
			removed->order = buddy->order = free_order - 1;
			zone->free_lists[free_order] = dlist_get_next(zone->free_lists[free_order], list);
			dlist_set_next(removed, list, buddy);
			dlist_set_next(buddy, list, zone->free_lists[free_order - 1]);
			zone->free_lists[free_order - 1] = removed;
		}
	}
	struct page *head = zone->free_lists[order];
	kassert(head->order == (int8_t)order);
	zone->free_lists[order] = dlist_get_next(head, list);
	dlist_set_next(head, list, (struct page *)NULL);
	dlist_set_prev(head, list, (struct page *)NULL);
	//klog("pmm", "Allocated order %u at %p\n", order, page_to_virt(head));
	kassert_dbg(head != NULL);
	return head;
}

struct page *pmm_alloc_order(unsigned int order, unsigned int alloc_flags)
{
	kassert(order < MAX_ORDER);
	spin_lock(&zone_list_lock);
	slist_foreach(zone, list, zone_list) {
		// TODO: Early check zone with page count before alloc
		struct page *rv = zone_alloc_order(zone, order, alloc_flags);
		if (rv != NULL) {
			spin_unlock(&zone_list_lock);
			return rv;
		}
	}
	spin_unlock(&zone_list_lock);
	if (!(alloc_flags & GFP_CAN_FAIL))
		panic("PMM out of memory");
	return NULL;
}

// TODO: Make this not O(n) somehow
static struct zone *page_to_zone(struct page *page)
{
	physaddr_t pa = page_to_phys(page);
	slist_foreach(zone, list, zone_list) {
		if (pa >= zone->pa_start && pa < (zone->pa_start + zone->len))
			return zone;
	}
	return NULL;
}

static bool is_page_free(struct page *page, struct zone *zone)
{
	if (dlist_get_next(page, list) == NULL && dlist_get_prev(page, list) == NULL)
		return false;
	for (size_t i = 0; i < MAX_ORDER; i++)
		if (zone->free_lists[i] == page)
			return false;
	return true;
}

static void __pmm_free_order(struct page *page, unsigned int order, struct zone *zone)
{
	struct page *buddy = page_buddy(page, order);
	struct page *first = (void *)MIN((uintptr_t)buddy, (uintptr_t)page);
	// If buddy is free, merge
	// If buddy is not free, insert page back into list
	if (is_page_free(buddy, zone) && buddy->order == (int8_t)order) {
		// Remove buddy from list order
		struct page *prev = dlist_get_prev(buddy, list);
		struct page *next = dlist_get_next(buddy, list);
		if (prev != NULL)
			dlist_set_next(prev, list, next);
		if (next != NULL)
			dlist_set_prev(next, list, prev);
		// Insert first into list order + 1
		//klog("pmm", "Merging blocks of order %u\n", order);
		__pmm_free_order(first, order + 1, zone);
	} else {
		dlist_set_next(page, list, zone->free_lists[order]);
		zone->free_lists[order] = page;
		page->order = order;
		//klog("pmm", "Freed order %u for page %p\n", order, page_to_virt(page));
	}
}

void pmm_free_order(struct page *page, unsigned int order)
{
	kassert_dbg(page != NULL);
	kassert_dbg(order < MAX_ORDER);
	kassert_dbg(dlist_get_next(page, list) == NULL);
	kassert_dbg(dlist_get_prev(page, list) == NULL);
	spin_lock(&zone_list_lock);
	struct zone *zone = page_to_zone(page);
	kassert(zone != NULL);
	__pmm_free_order(page, order, zone);
	spin_unlock(&zone_list_lock);
}


