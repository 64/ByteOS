#include "mm.h"
#include "libk.h"
#include "ds/linked.h"

void area_add(struct mmu_info *mmu, struct vm_area *area)
{
	kassert_dbg(mmu != &kernel_mmu);
	if (mmu->areas == NULL) {
		mmu->areas = area;
	} else {
		struct vm_area *prev = NULL;
		slist_foreach(cur, list, mmu->areas) {
			kassert_dbg(cur->base != area->base);
			if (cur->base > area->base) {
				kassert_dbg(area->base + area->len < cur->base);
				slist_set_next(area, list, cur);
				if (prev != NULL)
					slist_set_next(prev, list, area);
				else {
					mmu->areas = area;
					slist_set_next(area, list, cur);
				}
				return;
			}
			prev = cur;
		}
		kassert_dbg(prev->base + prev->len < area->base);
		slist_set_next(prev, list, area);
	}
}
