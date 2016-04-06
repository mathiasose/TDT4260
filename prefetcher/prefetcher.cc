/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "cmath"
#include "interface.hh"

#define MAX_LENGTH 512
#define MAX_LOOKBACK 256

struct GHBEntry {
    GHBEntry(Addr address, GHBEntry * prev);
    Addr address;
    GHBEntry * prevOnIndex;
    GHBEntry * prev;
    GHBEntry * next;
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

GHBEntry::GHBEntry(Addr address, GHBEntry * prev) : address(address), prevOnIndex(prev) {}

GHB::GHB(IndexTable * iTable) : length(0), iTable(iTable), first(NULL), last(NULL) {}

void GHB::push(AccessStat stat) {
    IndexTableEntry * index = iTable->get(stat.pc);
    GHBEntry * prevOnIndex;
    if (index != NULL) {
        prevOnIndex = index->lastAccess;
    } else {
        index = new IndexTableEntry(stat.pc);
        iTable->push(index);
        prevOnIndex = NULL;
    }
    GHBEntry * newEntry = new GHBEntry(stat.mem_addr, prevOnIndex);
    index->lastAccess = newEntry;
    if (length==0) {
        first = newEntry;
    }
    last->next = newEntry;
    newEntry->prev = last;
    newEntry->next = NULL;
    last = newEntry;
    length++;
    if(length > MAX_LENGTH) {
        this->shift();
    }
}

void GHB::shift(){
    GHBEntry * trash = first;
    first = first->next;
    first->prev = NULL;
    length--;
    delete trash;
}

IndexTableEntry::IndexTableEntry(Addr pc) : pc(pc) {}

IndexTable::IndexTable() : length(0), first(NULL), last(NULL){}

void IndexTable::push(IndexTableEntry* entry) {
    if (length == 0) {
        first = entry;
    }
    entry->prev = last;
    last = entry;
    length++;
}

IndexTableEntry* IndexTable::get(Addr pc) {
    IndexTableEntry * current = first;
    while (true) {
        if (current->pc == pc) {
            return current;
        } else if (current->prev == NULL) {
            return NULL;
        } else {
            current = current->prev;
        }
    }
}

IndexTable iTable;
GHB history(&iTable);

int delta(Addr pc) {
    int delta1, delta2, nextDelta1, nextDelta2;
    GHBEntry * current = iTable.get(pc)->lastAccess;
    if (current->prevOnIndex == NULL || current->prevOnIndex->prevOnIndex == NULL) {
        return BLOCK_SIZE;
    }
    delta1 = current->address - current->prevOnIndex->address;
    delta2 = current->prevOnIndex->address - current->prevOnIndex->prevOnIndex->address;
    for (int i = 0;i<MAX_LOOKBACK;i++) {
        current = current->prevOnIndex->prevOnIndex;
        if (current->prevOnIndex == NULL || current->prevOnIndex->prevOnIndex == NULL) {
            return BLOCK_SIZE;
        }
        
        nextDelta1 = current->address - current->prevOnIndex->address;
        nextDelta2 = current->prevOnIndex->address - current->prevOnIndex->prevOnIndex->address;
        if ((delta1 == nextDelta1) && (delta2 ==  nextDelta2)) {
            delta1 = nextDelta1;
            delta2 = nextDelta2;
        } else {
            return delta2;
        }
        
    }
    return delta2;
}

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

void prefetch_access(AccessStat stat)
{
    history.push(stat);
    try_prefetch(stat.mem_addr + delta(stat.pc));
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
