/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "cmath"
#include "interface.hh"

#define MAX_LENGTH 512

struct GHBEntry {
    GHBEntry(Addr address);
    Addr address
    GHBEntry * prevOnIndex;
    GHBEntry * prevInGHB;
    GHBEntry * next;
};

GHBEntry::GHBEntry(Addr address, GHBEntry * prev) : address(address), prevOnIndex(prev) {}

struct GHB {
    GHB();
    void shift();
    void push(AccessStat stat);
    int length;
    GHBEntry * first;
    GHBEntry * last;
};

GHB::GHB() : : length(0), first(NULL), last(NULL){}


void GHB::push(AccessStat stat) {
    
    if (indexTable.has(stat.pc)) {
        indexTableEntry* index = indexTable.get(stat.pc)
        GHBEntry * prevOnIndex = index->lastAccess;
    } else {
        indexTableEntry index =  new indexTableEntry(stat.pc)
        indexTable.push(index)
        GHBEntry prevOnIndex = NULL
    }
    GHBEntry newEntry = new GHBEntry(stat.mem_addr,prevOnIndex)
    index->lastAccess = newEntry;
    if (length==0) {
        first = newEntry
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
    GHBEntry* trash = first;
    first = first->next;
    first->prev = NULL;
    this->length--;
    delete trash;
}

struct indexTableEntry {
    indexTableEntry(Addr pc);
    Addr pc;
    indexTableEntry * prev;
    GHBEntry * lastAccess;
};

indexTableEntry::indexTableEntry(Addr pc) : pc(pc) {}


struct indexTable {
    indexTable();
    void push(indexTableEntry* entry);
    void shift();
    bool has(Addr pc);
    indexTableEntry get(Addr pc);
    indexTableEntry * first;
    indexTableEntry * last;
    int length;
};

indexTable::indexTable() : length(0), first(NULL), last(NULL){}

void indexTable::push(indexTableEntry* entry) {
    if (length==0) {
        first = entry;
    }
    entry->prev = last;
    last = entry;
    
}

struct deltaTable {
    deltaTable();
};


GHB ghb;
indexTable index;


void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    ghb = new GHB();
    index = new indexTable();

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
