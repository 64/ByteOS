#include <memory/kheap.h>
#include <memory/paging.h>
#include <klog.h>

extern uint32_t end;
uintptr_t placement_address = (uintptr_t)&end;

kheap_heap *kheap = NULL;

static int32_t kheap_find_smallest(uint32_t size, bool page_align, kheap_heap *heap) {
	uint32_t iter = 0;
	while (iter < heap->index.size) {
		kheap_header *header = (kheap_header*)oarray_lookup(iter, &heap->index);
		if (page_align) {
			uint32_t location = (uint32_t)header;
			int32_t offset = 0;
			if (((location + sizeof(kheap_header)) & 0xFFFFF000) != 0)
				offset = PAGE_SIZE - (location + sizeof(kheap_header)) % PAGE_SIZE;
			int32_t hole_size = (int32_t)header->size - offset;
			if (hole_size >= (int32_t)size)
				break;
		} else if (header->size >= size)
			break;
		iter++;
	}

	if (iter == heap->index.size)
		return -1;
	else
		return iter;
}

static bool kheap_less_than(void *a, void *b) {
	return (((kheap_header *)a)->size < ((kheap_header *)b)->size) ? 1 : 0;
}

static void kheap_expand(uint32_t new_size, kheap_heap *heap) {
	klog_assert(new_size > heap->end_addr - heap->start_addr);
	// Get the nearest following page boundary.
	if ((new_size & 0xFFFFF000) != 0) {
	    	new_size &= 0xFFFFF000;
	    	new_size += PAGE_SIZE;
	}
	// Make sure we are not overreaching ourselves.
	klog_assert(heap->start_addr + new_size <= heap->max_addr);

	// This should always be on a page boundary.
	uint32_t old_size = heap->end_addr - heap->start_addr;
	uint32_t i = old_size;
	while (i < new_size) {
		paging_alloc_frame(
			paging_get(heap->start_addr + i, 1, kernel_directory),
			(heap->supervisor) ? 1 : 0,
			(heap->readonly) ? 0 : 1,
			0
		);
		i += PAGE_SIZE;
	}
	heap->end_addr = heap->start_addr + new_size;
}

static uint32_t kheap_contract(uint32_t new_size, kheap_heap *heap) {
	klog_assert(new_size < heap->end_addr - heap->start_addr);

	if (new_size & 0x1000) {
		new_size &= PAGE_SIZE;
		new_size += PAGE_SIZE;
	}

	if (new_size < KHEAP_MIN_SIZE)
		new_size = KHEAP_MIN_SIZE;

	uint32_t old_size = heap->end_addr - heap->start_addr;
	uint32_t i = old_size - PAGE_SIZE;
	while (new_size < i) {
		paging_free_frame((uintptr_t)paging_get(heap->start_addr + i, 0, kernel_directory));
		i -= PAGE_SIZE;
	}

	heap->end_addr = heap->start_addr + new_size;
	return new_size;
}

void *kheap_alloc(uint32_t size, bool page_align, kheap_heap *heap) {
	uint32_t new_size = size + sizeof(kheap_header) + sizeof(kheap_footer);
	int32_t iter = kheap_find_smallest(new_size, page_align, heap);

	if (iter == -1) {
		uint32_t old_length = heap->end_addr - heap->start_addr;;
		uint32_t old_end_address = heap->end_addr;

		kheap_expand(old_length + new_size, heap);
		uint32_t new_length = heap->end_addr - heap->start_addr;

		iter = 0;
		uint32_t idx = ~0; uint32_t value = 0;
		while ((uint32_t)iter < heap->index.size) {
			uint32_t tmp = (uint32_t)oarray_lookup(iter, &heap->index);
			if (tmp > value) {
				value = tmp;
				idx = iter;
			}
			iter++;
		}

		if (idx == (uint32_t)~0) {
			kheap_header *header = (kheap_header *)old_end_address;
			header->magic = KHEAP_MAGIC;
			header->size = new_length - old_length;
			header->is_hole = 1;
			kheap_footer *footer = (kheap_footer *)(old_end_address + header->size - sizeof(kheap_footer));
			footer->magic = KHEAP_MAGIC;
			footer->header = header;
			oarray_insert((void*)header, &heap->index);
		} else {
			kheap_header *header = oarray_lookup(idx, &heap->index);
			header->size += new_length - old_length;
			kheap_footer *footer = (kheap_footer *) ((uintptr_t)header + header->size - sizeof(kheap_footer));
			footer->header = header;
			footer->magic = KHEAP_MAGIC;
		}

		return kheap_alloc(size, page_align, heap);
	}

	kheap_header *orig_hole_header = (kheap_header *)oarray_lookup(iter, &heap->index);
	uintptr_t orig_hole_pos = (uintptr_t)orig_hole_header;
	uint32_t orig_hole_size =  orig_hole_header->size;

	if (orig_hole_size < (sizeof(kheap_header) + sizeof(kheap_footer))) {
		size += orig_hole_size - new_size;
		new_size = orig_hole_size;
	}

	if (page_align && (orig_hole_pos & 0xFFFFF000)) {
		uintptr_t new_location = orig_hole_pos + PAGE_SIZE \
		 			 - (orig_hole_pos & 0xFFF) - sizeof(kheap_header);
		kheap_header *hole_header = (kheap_header*)orig_hole_pos;
		hole_header->size = PAGE_SIZE - (orig_hole_pos & 0xFFF) - sizeof(kheap_header);
		hole_header->magic = KHEAP_MAGIC;
		hole_header->is_hole = 1;
		kheap_footer *hole_footer = (kheap_footer*)((uint32_t)new_location - sizeof(kheap_footer));
		hole_footer->magic = KHEAP_MAGIC;
		hole_footer->header = hole_header;
		orig_hole_pos = new_location;
		orig_hole_size = orig_hole_size - hole_header->size;
	} else
		oarray_remove(iter, &heap->index);

	kheap_header *block_header = (kheap_header*)orig_hole_pos;
	block_header->magic = KHEAP_MAGIC;
	block_header->size = new_size;
	block_header->is_hole = 0;

	kheap_footer *block_footer = (kheap_footer*)(orig_hole_pos + sizeof(kheap_header) + size);
	block_footer->magic = KHEAP_MAGIC;
	block_footer->header = block_header;

	if (orig_hole_size - new_size > 0) {
		kheap_header *hole_header = (kheap_header*)(orig_hole_pos + sizeof(kheap_header) + size + sizeof(kheap_footer));
		hole_header->magic    = KHEAP_MAGIC;
		hole_header->is_hole  = 1;
		hole_header->size     = orig_hole_size - new_size;
		kheap_footer *hole_footer = (kheap_footer *)((uint32_t)hole_header + orig_hole_size - new_size - sizeof(kheap_header));
		if ((uint32_t)hole_footer < heap->end_addr) {
		    hole_footer->magic = KHEAP_MAGIC;
		    hole_footer->header = hole_header;
		}
		oarray_insert((void*)hole_header, &heap->index);
	}

	return (void *)((uintptr_t)block_header + sizeof(kheap_header));
}

kheap_heap *kheap_create(uintptr_t start, uintptr_t end, uintptr_t max, bool supervisor, bool readonly) {
	kheap_heap *heap = (kheap_heap*)kmalloc(sizeof(kheap_heap));
	klog_assert(start % PAGE_SIZE == 0);
	klog_assert(end % PAGE_SIZE == 0);
	heap->index = oarray_place((void*)start, KHEAP_INDEX_SIZE, &kheap_less_than);
	start += sizeof(void*) * KHEAP_INDEX_SIZE;

	if ((start & 0xFFFFF000) != 0) {
		start &= 0xFFFFF000;
		start += PAGE_SIZE;
	}

	heap->start_addr = start;
	heap->end_addr = end;
	heap->max_addr = max;
	heap->supervisor = supervisor;
	heap->readonly = readonly;

	kheap_header *hole = (kheap_header *)start;
	hole->size = end - start;
	hole->magic = KHEAP_MAGIC;
	hole->is_hole = 1;
	oarray_insert((void *)hole, &heap->index);

	return heap;
}

void kheap_free(void *p, kheap_heap *heap) {
	if (p == NULL)
		return;

	kheap_header *header = (kheap_header*)((uintptr_t)p - sizeof(kheap_header));
	kheap_footer *footer = (kheap_footer*)((uintptr_t)header + header->size - sizeof(kheap_footer));

	klog_assert(header->magic == KHEAP_MAGIC);
	klog_assert(footer->magic == KHEAP_MAGIC);

	header->is_hole = 1;
	bool do_add = 1;

	kheap_footer *test_footer = (kheap_footer*)((uintptr_t)header - sizeof(kheap_footer));
	if (test_footer->magic == KHEAP_MAGIC && test_footer->header->is_hole == 1) {
		uint32_t cache_size = header->size;
		header = test_footer->header;
		footer->header = header;
		header->size += cache_size;
		do_add = 0;
	}

	kheap_header *test_header = (kheap_header*)((uintptr_t)footer + sizeof(kheap_footer));
	if (test_header->magic == KHEAP_MAGIC && test_header->is_hole == 1) {
		header->size += test_header->size;
		test_footer = (kheap_footer*)((uintptr_t)test_header + test_header->size - sizeof(kheap_footer));
		footer = test_footer;
		uint32_t iter = 0;
		while ((iter < heap->index.size) && (oarray_lookup(iter, &heap->index) != (void*)test_header))
			iter++;

		klog_assert(iter < heap->index.size);
		oarray_remove(iter, &heap->index);
	}

	if ((uintptr_t)footer + sizeof(kheap_footer) == heap->end_addr) {
		uint32_t old_length = heap->end_addr - heap->start_addr;
		uint32_t new_length = kheap_contract((uintptr_t)header - heap->start_addr, heap);

		if (header->size - (old_length - new_length)) {
			header->size -= old_length - new_length;
			footer = (kheap_footer*)((uintptr_t)header + header->size - sizeof(kheap_footer));
			footer->magic = KHEAP_MAGIC;
			footer->header = header;
		} else {
			uint32_t iter = 0;
			while ((iter < heap->index.size) && (oarray_lookup(iter, &heap->index) != (void*)test_header))
				iter++;
			if (iter < heap->index.size)
				oarray_remove(iter, &heap->index);
		}
	}

	if (do_add == 1)
		oarray_insert((void*)header, &heap->index);
}

uintptr_t kmalloc_internal(uint32_t size, bool align, uint32_t *phys) {
	uintptr_t temp;
	if (kheap != NULL) {
		temp = (uintptr_t)kheap_alloc(size, align, kheap);
		if (phys != NULL) {
			uint32_t *page = paging_get(temp, 0, kernel_directory);
			*phys = ((*page & 0xFFFFF000) >> 12) * PAGE_SIZE + (temp & 0xFFF);
		}
	} else {
		if (align == 1 && (placement_address & 0xFFFFF000)) {
			placement_address &= 0xFFFFF000;
			placement_address += PAGE_SIZE;
		}

		if (phys != NULL)
			*phys = placement_address;

		temp = placement_address;
		placement_address += size;
	}
	return temp;
}

uintptr_t kmalloc_a(uint32_t size) {
	return kmalloc_internal(size, 1, NULL);
}

uintptr_t kmalloc_p(uint32_t size, uint32_t *phys) {
	return kmalloc_internal(size, 0, phys);
}

uintptr_t kmalloc_ap(uint32_t size, uint32_t *phys) {
	return kmalloc_internal(size, 1, phys);
}

uintptr_t kmalloc(uint32_t size) {
	return kmalloc_internal(size, 0, NULL);
}

void kfree(void *p) {
	return kheap_free(p, kheap);
}
