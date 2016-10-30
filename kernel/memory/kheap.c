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
static struct bitset heap_aa;

#define HEAP_AA_INDEX(x) ((((uintptr_t)(x)) - KHEAP_START) / PAGE_SIZE)

static kheap_hdr *kheap_morecore(size_t num_units) {
	kheap_hdr *up;

	if (num_units < MIN_MORECORE_UNITS)
		num_units = MIN_MORECORE_UNITS;

	virt_addr iter;
	for (iter = heap_end; iter < heap_end + (HDR_SIZE * num_units); iter += PAGE_SIZE) {
		klog_detail("Iter: 0x%x\n", iter);
		pmm_alloc_frame(iter, 0);
	}
	virt_addr temp = heap_end;
	heap_end = iter;
	up = (kheap_hdr*)temp;
	up->s.size = num_units;
	kheap_free((void*)(up + 1));
	klog_detail("Morecore new memory at: 0x%x\n", freep->s.size);
	return freep;
}

void kheap_init() {
	if (freep == NULL) {
		bitset_create(&heap_aa, ((KHEAP_AA_MAX - KHEAP_AA_START) / PAGE_SIZE / 8) || 1);
		base.s.ptr = (freep = &base);
		base.s.size = 0;
	}
}

static void *kheap_alloc_pa(size_t size) {
	if (KHEAP_AA_START + size > KHEAP_AA_MAX)
		goto err;
	size_t out = 0;
	if (!bitset_find_first(&heap_aa, &out))
		goto err;

	virt_addr res = KHEAP_AA_START + (out * PAGE_SIZE);
	pmm_alloc_frame(res, 0);
	bitset_set(&heap_aa, out);
	return (void*)res;
err:
	klog_panic("Kheap page aligned area full");
	return NULL;
}

static bool kheap_free_pa(void *addr) {
	if ((uintptr_t)addr > KHEAP_AA_START && (uintptr_t)addr < KHEAP_AA_MAX) {
		bitset_clear(&heap_aa, HEAP_AA_INDEX(addr));
		return 1;
	}
	return 0;
}

void *kheap_alloc(size_t size, bool page_align) {
	kheap_hdr *p, *prevp;
	uint32_t num_units;

	if (page_align)
		return kheap_alloc_pa(size);

	if (heap_end + size > KHEAP_MAX)
		klog_panic("Kheap full");

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

void kheap_free(void *ap) {
	if (kheap_free_pa(ap))
		return;

	kheap_hdr *bp, *p;
	bp = (kheap_hdr *)ap - 1;
	for (p = freep; !((p < bp) && (bp < p->s.ptr)); p = p->s.ptr)
		if (p >= p->s.ptr && (p < bp || bp < p->s.ptr))
			break;

	if ((bp + bp->s.size) == p->s.ptr) {
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	} else
		bp->s.ptr = p->s.ptr;

	if (p + p->s.size == bp) {
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	} else
		p->s.ptr = bp;
	freep = p;

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

void *kmalloc_a(size_t size) {
	return (void*)kmalloc_internal(size, 1, NULL);
}

void *kmalloc_p(size_t size, phys_addr *phys) {
	return (void*)kmalloc_internal(size, 0, phys);
}

void *kmalloc_ap(uint32_t size, phys_addr *phys) {
	return (void*)kmalloc_internal(size, 1, phys);
}

void *kmalloc(uint32_t size) {
	return (void*)kmalloc_internal(size, 0, NULL);
}

void kfree(void *p) {
	return freep ? kheap_free(p) : NULL;
}
