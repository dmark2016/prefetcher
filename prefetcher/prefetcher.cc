/*
 * Simple implementation of SDP.
 */

#include <stdlib.h>
#include "interface.hh"

#define SIZE 1000

struct Entry
{
	Addr pc;
	Addr last;
	int valid;
};

Entry sdptable[SIZE] = {{ 0 }};

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
	int index = stat.pc % SIZE;

	if (sdptable[index].pc == 0 || sdptable[index].pc != stat.pc)
	{
		struct Entry e;
		e.pc = stat.pc;
		e.last = stat.mem_addr;
		e.valid = 1;
		sdptable[index] = e;
	}
	else
	{
		int stride = abs(sdptable[index].last - stat.mem_addr);
		sdptable[index].last = stat.mem_addr;

		if (!in_cache(stat.mem_addr + stride) && stat.mem_addr + stride <= MAX_PHYS_MEM_ADDR) {
			issue_prefetch(stat.mem_addr + stride);
		}
		if (!in_cache(stat.mem_addr + stride) && stat.mem_addr + 2 * stride <= MAX_PHYS_MEM_ADDR) {
			issue_prefetch(stat.mem_addr + 2 * stride);
		}
		if (!in_cache(stat.mem_addr + stride) && stat.mem_addr + 3 * stride <= MAX_PHYS_MEM_ADDR) {
			issue_prefetch(stat.mem_addr + 3 * stride);
		}
	}
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
