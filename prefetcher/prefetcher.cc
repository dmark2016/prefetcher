/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#define DEGREE 1


void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
    Addr pf_addr = stat.mem_addr;

	for (int i = 0; i < DEGREE; i++) {
		pf_addr += BLOCK_SIZE;
		if (!in_cache(pf_addr)) {
			issue_prefetch(pf_addr);
		}
	}
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
