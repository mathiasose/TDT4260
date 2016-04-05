/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "cmath"
#include "interface.hh"

#define MAX_LENGTH 512

struct GHBEntry {
    GHBEntry(Addr address, GHBEntry * prev);
    Addr address;
    GHBEntry * prevOnIndex;
    GHBEntry * prevInGHB;
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

struct deltaTable {
    deltaTable();
};


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
    first->prevInGHB = NULL;
    length--;
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
