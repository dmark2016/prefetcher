/*
 * Simple implementation of SDP.
 */

#include <stdlib.h>
#include "interface.hh"

#define SIZE 1000
#define DEGREE 7

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
		int stride =  stat.mem_addr - sdptable[index].last;
		sdptable[index].last = stat.mem_addr;

        for (unsigned i = 0; i < DEGREE; i++) {
            Addr a = stat.mem_addr + (i + 1) * stride;
            if (!in_cache(a) && a <= MAX_PHYS_MEM_ADDR) {
                issue_prefetch(a);
            }
        }
	}
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
