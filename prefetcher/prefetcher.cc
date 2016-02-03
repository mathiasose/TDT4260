/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"

struct Request {
    Request(Addr addr, bool ms, Request *prv);
    bool miss;
    Addr adress;
    int strideToPrev;
    Request * next, * prev;
};

Request::Request(Addr addr, bool ms, Request *prv) : adress(addr), miss(ms), prev(prv), next(NULL){}

struct List {
    List();
    Request * first, * last;
};

List::List() : first(NULL), last(NULL){}

static List * list;


void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */
    list = new List();

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
    /* pf_addr is now an address within the _next_ cache block */
    Request * req = new Request(stat.mem_addr,stat.miss,list->last);
    
    if (!(list->first)){
        list->first = req;
        list->last = req;
    } else {
        list->last->next = req;
        list->last = req;
        list->last->strideToPrev = list->last->prev->adress - list->last->adress;
    }
    
    Addr pf_addr = stat.mem_addr + list->last->strideToPrev;
    
    

    /*
     * Issue a prefetch request if a demand miss occured,
     * and the block is not already in cache.
     */
    if (stat.miss && !in_cache(pf_addr) && in_mshr_queue(pf_addr)) {
        issue_prefetch(pf_addr);
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
