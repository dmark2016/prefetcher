/**
 * A simple global history buffer implementation with delta correlation.
 */

#include "interface.hh"

#define HISTORY_SIZE 700
#define INDEX_SIZE 90
#define DELTA_SIZE 10
#define DEGREE 7

// Must be able to contain HISTORY_SIZE
typedef unsigned int index_t;

typedef struct {
    Addr addr;
    index_t previous : 10;
    index_t next : 10;
} history_entry; // 84 bits

typedef struct {
    Addr pc;
    index_t last : 10;
} index_entry; // 74 bit

index_entry pc_index[INDEX_SIZE];
history_entry history[HISTORY_SIZE];

index_t end;


/**
 * Perfetches an address if it is valid and not in cache.
 */
void try_prefetch(Addr addr) {
    if (addr > 0 && addr < MAX_PHYS_MEM_ADDR && !in_cache(addr)) {
        issue_prefetch(addr);
    }
}

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

    // Clever approach - delta correlation
    for (int i = 2; i < n_deltas; i++) {
        if (deltas[i - 1] == deltas[0] && deltas[i] == deltas[1]) {

            //MATCH! Prefetch
            for (int j = 0; j < DEGREE; j++) {
                base += deltas[i - (j % (i - 1)) - 2];
                try_prefetch(base);
            }

            return;
        }
    }

    // A little less clever approach - RPT/SDP
    if (n_deltas == 1 || (n_deltas > 1 && deltas[0] == deltas[1])) {
        for (int i = 0; i < DEGREE; i++) {
            base += deltas[0];
            try_prefetch(base);
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
