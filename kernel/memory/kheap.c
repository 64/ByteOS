#include <memory/kheap.h>
#include <memory/paging.h>
#include <klog.h>

extern uint32_t end;
uintptr_t placement_address = (uintptr_t)&end;

static int32_t kheap_find_smallest(uint32_t size, uint8_t page_align, kheap_heap *heap) {
	uint32_t iter = 0;
	while (iter < heap->index.size) {
		kheap_header *header = (kheap_header*)oarray_lookup(iter, &heap->index);
		if (page_align > 0) {
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
			(heap->readonly) ? 0 : 1
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

void *kheap_alloc(uint32_t size, uint8_t page_align, kheap_heap *heap) {
	(void)size; (void)page_align; (void)heap;
	return NULL;
}

void kheap_free(void *p, kheap_heap *heap) {
	(void)p; (void)heap;
}

uintptr_t kmalloc_internal(uint32_t size, bool align, uint32_t *phys) {
	if (align == 1 && (placement_address & 0xFFFFF000)) {
		placement_address &= 0xFFFFF000;
		placement_address += PAGE_SIZE;
	}

	if (phys != NULL)
		*phys = placement_address;

	uintptr_t temp = placement_address;
	placement_address += size;
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
