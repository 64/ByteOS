#include "smp.h"
#include "libk.h"
#include "ds/bitmap.h"

void cpuset_init(cpuset_t *cpus)
{
	*cpus = 0;	
}

bool cpuset_query_id(cpuset_t *cpus, uint8_t id)
{
	kassert_dbg(id < 8);
	return *cpus & (1 << id);
}

void cpuset_set_id(cpuset_t *cpus, uint8_t id, bool val)
{
	kassert_dbg(id < 8);
	if (val)
		*cpus |= (1 << id);
	else
		*cpus &= ~(1 << id);
}
