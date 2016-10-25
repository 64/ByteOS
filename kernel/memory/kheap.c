#include <memory/kheap.h>
#include <memory/pmm.h>
#include <memory/paging.h>
#include <algs/bitset.h>
#include <sys/util.h>
#include <string.h>
#include <klog.h>

static kheap_hdr base;
kheap_hdr *freep = NULL;
static virt_addr heap_end = KHEAP_START;

static kheap_hdr *kheap_morecore(size_t num_units) {
	kheap_hdr *up;

	if (num_units < MIN_MORECORE_UNITS)
		num_units = MIN_MORECORE_UNITS;

	virt_addr iter;
	for (iter = heap_end; iter < heap_end + (HDR_SIZE * num_units); iter += PAGE_SIZE) {
		pmm_alloc_frame(iter, 0);
	}
	virt_addr temp = heap_end;
	heap_end = iter;
	up = (kheap_hdr*)temp;
	up->s.size = num_units;
	kheap_free((void*)(up + 1));
	return freep;
}

void kheap_init() {
	if (freep == NULL) {
		base.s.ptr = (freep = &base);
		base.s.size = 0;
	}
}

static void *kheap_alloc_pa(size_t UNUSED(size)) {
	return NULL;
}

static bool kheap_free_pa(void * UNUSED(addr)) {
	return 0;
}

void *kheap_alloc(size_t size, bool page_align) {
	kheap_hdr *p, *prevp;
	uint32_t num_units;

	if (page_align)
		return kheap_alloc_pa(size);

	if ((prevp = freep) == NULL)
		klog_panic("Kernel heap has not been initialized");

	num_units = (size + HDR_SIZE - 1) / HDR_SIZE + 1;
	for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
		if (p->s.size >= num_units) {
			if (p->s.size == num_units)
				prevp->s.ptr = p->s.ptr;
			else {
				p->s.size -= num_units;
				p += p->s.size;
				p->s.size = num_units;
			}
			freep = prevp;
			return (void*)(p + 1);
		}
		if (p == freep)
			if ((p = kheap_morecore(num_units)) == NULL)
				return NULL; // Out of memory
	}
	return NULL;
}

void kheap_free(void *p) {
	if (kheap_free_pa(p))
		return;

}

extern uint32_t end;
phys_addr placement_address = (phys_addr)&end;

virt_addr kmalloc_internal(size_t size, bool page_align, phys_addr *phys) {
	uintptr_t temp;
	if (freep != NULL) {
		temp = (virt_addr)kheap_alloc(size, page_align);
		if (phys != NULL)
			*phys = virt_to_phys(temp);
	} else {
		if (page_align == 1 && (placement_address & 0xFFFFF000)) {
			// 4096 byte align the pointer
			placement_address &= 0xFFFFF000;
			placement_address += PAGE_SIZE;
		} else if (page_align == 0 && (placement_address & 0x8)) {
			// 8 byte align the pointer
			placement_address &= 0x8;
			placement_address += 8;
		}

		if (phys != NULL)
			*phys = placement_address;

		temp = placement_address;
		placement_address += size;
	}
	return temp;
}

virt_addr kmalloc_a(size_t size) {
	return kmalloc_internal(size, 1, NULL);
}

virt_addr kmalloc_p(size_t size, phys_addr *phys) {
	return kmalloc_internal(size, 0, phys);
}

virt_addr kmalloc_ap(uint32_t size, phys_addr *phys) {
	return kmalloc_internal(size, 1, phys);
}

virt_addr kmalloc(uint32_t size) {
	return kmalloc_internal(size, 0, NULL);
}

void kfree(void *p) {
	return kheap_free(p);
}
