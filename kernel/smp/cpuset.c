#include "smp.h"
#include "libk.h"
#include "ds/bitmap.h"

void cpuset_init(cpuset_t *cpus)
{
	*cpus = 0;	
}

void cpuset_copy(cpuset_t *dest, cpuset_t *src)
{
	*dest = *src;
}

bool cpuset_query_id(cpuset_t *cpus, cpuid_t id)
{
	kassert_dbg(id < MAX_CORES);
	return *cpus & (1 << id);
}

void cpuset_set_id(cpuset_t *cpus, cpuid_t id, bool val)
{
	kassert_dbg(id < MAX_CORES);
	if (val)
		*cpus |= (1 << id);
	else
		*cpus &= ~(1 << id);
}

#ifdef VERBOSE
void cpuset_dump(cpuset_t *cpus)
{
	for (cpuid_t i = 0; i < sizeof(cpuset_t) * 4; i++) {
		if (cpuset_query_id(cpus, i))
			kprintf("%u ", i);
	}
}
#endif
