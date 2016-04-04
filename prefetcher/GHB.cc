/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "cmath"
#include "interface.hh"

#define MAX_LENGTH 512

GHB history;
IndexTable index;


struct GHBEntry {
    GHBEntry(Addr address, GHBEntry * prev);
    Addr address;
    GHBEntry * prevOnIndex;
    GHBEntry * prevInGHB;
    GHBEntry * next;
};

struct GHB {
    GHB();
    void shift();
    void push(AccessStat stat);
    int length;
    GHBEntry * first;
    GHBEntry * last;
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
    bool has(Addr pc);
    IndexTableEntry get(Addr pc);
    IndexTableEntry * first;
    IndexTableEntry * last;
    int length;
};

struct deltaTable {
    deltaTable();
};


GHBEntry::GHBEntry(Addr address, GHBEntry * prev) : address(address), prevOnIndex(prev) {}

GHB::GHB() : length(0), first(NULL), last(NULL) {}

void GHB::push(AccessStat stat) {
    
    if (indexTable.has(stat.pc)) {
        IndexTableEntry * index = indexTable.get(stat.pc);
        GHBEntry * prevOnIndex = index->lastAccess;
    } else {
        IndexTableEntry * index = new IndexTableEntry(stat.pc);
        indexTable.push(index);
        GHBEntry * prevOnIndex = NULL;
    }
    GHBEntry * newEntry = new GHBEntry(stat.mem_addr, prevOnIndex);
    index->lastAccess = newEntry;
    if (length==0) {
        first = newEntry;
    }
    last->next = newEntry;
    newEntry->prevInGHB = last;
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
    this->length--;
    delete trash;
}

IndexTableEntry::IndexTableEntry(Addr pc) : pc(pc) {}

IndexTable::IndexTable() : length(0), first(NULL), last(NULL){}

void IndexTable::push(IndexTableEntry* entry) {
    if (length==0) {
        first = entry;
    }
    entry->prev = last;
    last = entry;
    
}

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    DPRINTF(HWPrefetch, "init");
}

void prefetch_access(AccessStat stat)
{
    
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
