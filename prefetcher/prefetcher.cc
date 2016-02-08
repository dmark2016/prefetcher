/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"

#define RTP_SIZE 100
#define INITIAL 0
#define TRAINING 1
#define PREFETCH 2

typedef struct entry {
    Addr pc;
    Addr last;
    int stride;
    int state;
} Entry;

Entry rpt[RTP_SIZE] = {{ 0 }};

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
    if (stat.miss) {
        int index = stat.mem_addr % RTP_SIZE;
        Entry e;
        if (rpt[index].pc == 0 or rpt[index].pc != stat.pc) {
            // New entry
            e = {stat.pc, stat.mem_addr, -1, INITIAL};
            rpt[index] = e;
        } else {
            // Subsequent miss
            e = rpt[index];
            switch (e.state) {
                case INITIAL:
                    {
                        e.stride = abs(e.last - stat.mem_addr);
                        e.last = stat.mem_addr;
                        break;
                    }
                case TRAINING:
                    {
                        int delta = abs(e.last - stat.mem_addr);
                        if (delta == e.stride) {
                            e.state = PREFETCH;
                        } else {
                            e.stride = delta;
                        }
                    }
            }
        }
        if (e.state == PREFETCH && !in_cache(stat.mem_addr + e.stride)) {
            issue_prefetch(stat.mem_addr + e.stride);
        }
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
