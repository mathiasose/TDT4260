/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "cmath"
#include "interface.hh"

#define MAX_LENGTH 512
#define MAX_LOOKBACK 256
#define PREFETCH_DEGREE 4

struct GHBEntry {
    GHBEntry(Addr address, GHBEntry * prev);
    Addr address;
    GHBEntry * prevOnIndex;
    GHBEntry * nextOnIndex;
    GHBEntry * prev;
    GHBEntry * next;
    uint64_t delta;
};

struct IndexTableEntry {
    IndexTableEntry(Addr pc);
    Addr pc;
    IndexTableEntry * prev;
    GHBEntry * lastAccess;
};

struct IndexTable {
    IndexTable();
    void push(IndexTableEntry* entry);
    void shift();
    IndexTableEntry* get(Addr pc);
    int length;
    IndexTableEntry * first;
    IndexTableEntry * last;
};

struct GHB {
    GHB(IndexTable * iTable);
    void shift();
    void push(AccessStat stat);
    int length;
    IndexTable * iTable;
    GHBEntry * first;
    GHBEntry * last;
};

/*
struct deltaTable {
    deltaTable();
    void updateDeltas();
    int nextDelta();
    int[] deltaList;
};
*/

GHBEntry::GHBEntry(Addr address, GHBEntry * prev) :
    address(address),
    prevOnIndex(prev), nextOnIndex(NULL),
    prev(NULL), next(NULL),
    delta(0)
{ }

GHB::GHB(IndexTable * iTable) : length(0), iTable(iTable), first(NULL), last(NULL) {}

void GHB::push(AccessStat stat) {
    IndexTableEntry * index;
    GHBEntry * newEntry;

    // Get the previous index entry
    index = iTable->get(stat.pc);
    if (index == NULL) {
        index = new IndexTableEntry(stat.pc);
        iTable->push(index);
    }

    // Add the new history entry to the GHB
    newEntry = new GHBEntry(stat.mem_addr, index->lastAccess);
    if (length == 0) {
        first = newEntry;
        last  = newEntry;
    } else {
        last->next = newEntry;
        newEntry->prev = last;
        last = newEntry;
    }
    length++;

    // Update the index
    if (index->lastAccess != NULL) {
        index->lastAccess->nextOnIndex = newEntry;
        index->lastAccess->delta = newEntry->address - index->lastAccess->address;
    }
    index->lastAccess = newEntry;

    // Delete the oldest history entry
    if(length > MAX_LENGTH) {
        shift();
    }
}

void GHB::shift(){
    GHBEntry * trash = first;
    first = first->next;
    first->prev = NULL;
    length--;
    delete trash;
}

IndexTableEntry::IndexTableEntry(Addr pc) : pc(pc), lastAccess(NULL) {}

IndexTable::IndexTable() : length(0), first(NULL), last(NULL){}

void IndexTable::push(IndexTableEntry* entry) {
    if (length == 0) {
        first = entry;
    }
    entry->prev = last;
    last = entry;
    length++;
}

IndexTableEntry * IndexTable::get(Addr pc) {
    IndexTableEntry * current = first;
    while (current != NULL && current->pc != pc) {
        current = current->prev;
    }
    return current;
}

IndexTable iTable;
GHB history(&iTable);

void try_prefetch(Addr pf_addr) {
    
    // Issue prefetch.
    if (pf_addr <= MAX_PHYS_MEM_ADDR
        && !in_cache(pf_addr)
        && !in_mshr_queue(pf_addr)) {
        issue_prefetch(pf_addr);
    }
}

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "init");
}


// Find the earliest occurrence in history with a matching delta pair
GHBEntry * find_delta_match(GHBEntry * entry, uint64_t delta1, uint64_t delta2) {
    uint64_t delta3;
    uint64_t delta4;

    // Check that we have enough history
    if (entry == NULL
            || entry->prevOnIndex == NULL
            || entry->prevOnIndex->prevOnIndex == NULL) {
        return NULL;
    }

    // Compute deltas
    delta3 = entry->address - entry->prevOnIndex->address;
    delta4 = entry->prevOnIndex->address - entry->prevOnIndex->prevOnIndex->address;

    if (delta1 == delta3 && delta2 == delta4) {
        return entry;
    } else {
        return find_delta_match(entry->prevOnIndex, delta1, delta2);
    }
}

bool is_delta_match(GHBEntry * e1, GHBEntry * e2) {
    uint64_t d1, d2, d3, d4;
    d1 = e1->delta;
    d2 = e1->nextOnIndex->delta;
    d3 = e2->delta;
    d4 = e2->nextOnIndex->delta;
    return (d1 == d3) && (d2 == d4);
}

void prefetch_access(AccessStat stat)
{
    Addr pf_addr;
    IndexTableEntry * index;
    GHBEntry * first;
    GHBEntry * match;

    history.push(stat);

    index = iTable.get(stat.pc);
    if (index == NULL) {
        return;
    }
    first = index->lastAccess;

    // Check that we have enough history
    if (first == NULL
            || first->prevOnIndex == NULL
            || first->prevOnIndex->prevOnIndex == NULL) {
        return;
    }

    // Find a delta match
    first = first->prevOnIndex->prevOnIndex;
    match = first->prevOnIndex;
    while (match != NULL && !is_delta_match(first, match)) {
        match = first->prevOnIndex;
    }

    pf_addr = stat.mem_addr;
    for (int i=0; i<PREFETCH_DEGREE && match != NULL; i++) {
        pf_addr += match->delta;
        try_prefetch(pf_addr);
        match = match->nextOnIndex;
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
