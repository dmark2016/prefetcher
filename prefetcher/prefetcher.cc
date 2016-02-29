/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include <stdlib.h>
#include "interface.hh"

#define RPT_SIZE 1024
#define INITIAL 0
#define TRAINING 1
#define PREFETCH 2

typedef struct entry {
    Addr pc;
    Addr last;
    int stride;
    int state;
} Entry;

Entry rpt[RPT_SIZE] = {{ 0 }};

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
    Entry e;
    int index = stat.pc % RPT_SIZE;
    if (rpt[index].pc == 0 or rpt[index].pc != stat.pc) {
        // New entry
        e = *((Entry*) malloc(sizeof(Entry)));
        e.pc = stat.pc;
        e.last = stat.mem_addr;
        e.stride = 0;
        e.state = INITIAL;
        rpt[index] = e;
    } else {
        // Subsequent miss
        e = rpt[index];
        switch (e.state) {
            case INITIAL:
                {
                    e.stride = stat.mem_addr - e.last;
                    e.state = TRAINING;
                    e.last = stat.mem_addr;
                    break;
                }
            case TRAINING:
                {
                    int delta = stat.mem_addr - e.last;
                    if (delta == e.stride) {
                        e.state = PREFETCH;
                    } else {
                        e.stride = delta;
                    }
                }
        }
        rpt[index] = e;
    }
    if (e.state == PREFETCH && !in_cache(stat.mem_addr + e.stride) && stat.mem_addr + e.stride < MAX_PHYS_MEM_ADDR) {
        issue_prefetch(stat.mem_addr + e.stride);
    }
} 
void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
