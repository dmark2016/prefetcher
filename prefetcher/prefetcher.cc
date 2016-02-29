/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"

#define HISTORY_SIZE 255
#define INDEX_SIZE 64
#define DELTA_SIZE 10
#define PREFETCH_DEGREE 10

// Must be able to contain HISTORY_SIZE
typedef unsigned char index_t;

typedef struct {
    Addr addr;
    index_t previous;
    index_t next;
} history_entry;

typedef struct {
    Addr pc;
    index_t last;
} index_entry;

index_entry pc_index[INDEX_SIZE];
history_entry history[HISTORY_SIZE];

index_t end;

/**
 * Performs prefetches inferred from the history starting at the given entry.
 *
 * @param entry History entry to start at
 */
void infer_prefetches(history_entry * entry) {

    Addr base = entry->addr;

    int deltas[DELTA_SIZE];
    int n_deltas = 0;

    while (entry->previous != HISTORY_SIZE && n_deltas < DELTA_SIZE) {
        deltas[n_deltas] = entry->addr - history[entry->previous].addr;
        entry = history + entry->previous;
        n_deltas++;
    }

    if (n_deltas < 3) {
        //TODO: Any better approach in this case? Simple stride?
        return;
    }


    // Find this delta in the history table
    for (int i = 1; i < n_deltas; i++) {
        if (deltas[i] == deltas[0] && deltas[i + 1] == deltas[1]) {

            //MATCH! Prefetch
            for (int j = 1; j <= PREFETCH_DEGREE && j < i; j++) {
                base += deltas[i - j];

                if (base > 0 && base < MAX_PHYS_MEM_ADDR && !in_cache(base)) {
                    issue_prefetch(base);
                }
            }

            break;
        }
    }

}


/**
 * Update the history and index buffers.
 *
 * @param pc PC address of load instruction
 * @param addr Memory address requested
 * @return New history entry created
 */
history_entry * update_history(Addr pc, Addr addr) {

    // Calculate next history entry
    end = (end + 1) % HISTORY_SIZE;
    history_entry * h_entry = history + end;

    // Re-initialize history entry
    if (h_entry->next != HISTORY_SIZE) {
        history[h_entry->next].previous = HISTORY_SIZE;
    }

    h_entry->addr = addr;
    h_entry->next = HISTORY_SIZE;
    h_entry->previous = HISTORY_SIZE;

    // Update index entry
    index_entry * i_entry = pc_index + (pc % INDEX_SIZE);

    if (i_entry->pc != pc) {
        i_entry->pc = pc;
        i_entry->last = end;
        return h_entry;
    }

    // Link the new history entry
    h_entry->previous = i_entry->last;
    history[i_entry->last].next = end; //TODO: Check index?
    i_entry->last = end;

    return h_entry;
}



void prefetch_init(void)
{
    memset(&pc_index, 0, sizeof(pc_index));
    memset(&history, 0, sizeof(history));
    end = 0;

    //TODO: Better approach
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i].next = HISTORY_SIZE;
    }

    DPRINTF(HWPrefetch, "Initialized global-history-buffer prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
    history_entry * h_entry = update_history(stat.pc, stat.mem_addr);
    infer_prefetches(h_entry);
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
