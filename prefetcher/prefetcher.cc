/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"

static const int MAX_LENGTH = 200;
static const int CONSECUTIVE_STRIDES = 3;


struct Request {
    Request(Addr addr, bool ms, Request *prv);
    bool miss;
    Addr adress;
    int strideToPrev;
    //int index;
    Request * next, * prev;
};

Request::Request(Addr addr, bool ms, Request *prv) : adress(addr), miss(ms), prev(prv), next(NULL){}

struct List {
    List();
    void push(Request* req);
    //int getStrides(int len);
    void shift();
    bool judge();
    
    //int strides[MAX_LENGTH];
    int length;
    Request * first, * last;
};

List::List() : first(NULL), last(NULL), length(0){}

void List::push(Request* req) {
    this->last->next = req;
    this->last = req;
    this->last->strideToPrev = this->last->prev->adress - this->last->adress;
    this->last->next = NULL;
    //this->last->index = this->last->prev->index +1;
    this->length++;
    
    if(this->length > MAX_LENGTH) {
        this->shift();
    }
    
    //this->strides[index] = this->last->strideToPrev;

}

void List::shift(){
    Request* trash = this->first;
    this->first = this->first->next;
    this->length--;
    //decrement indexes
    /*
    int i;
    Request* currentReq = this->last;
    for(i=1;i<=this->length;i++) {
        currentReq->index = this->length -i;
        currentReq = currentReq->prev;
        this->strides[i-1] = this->strides[i];
    }*/
    delete trash;
}
/*
int List::getStrides(int len) {
    //get array of all the strides from last to <len> elements back in the array
    int i;
    int strides[len];
    Request* currentReq = this->last;
    for(i = 0;i<len;i++){
        strides[i] = currentReq->strideToPrev;
        currentReq = currentReq->prev;
    }
    return strides;
}
*/
bool List::judge() {
    int i;
    Request* currentReq = this->last;
    for (i=0; i<CONSECUTIVE_STRIDES; i++) {
        if (currentReq->strideToPrev == currentReq->prev->strideToPrev) {
            continue;
        } else {
            return false;
        }
        currentReq = currentReq->prev;
    }
    return true;
}



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
        //list->last->index = 0;
    } else {
        list->push(req);
    }
    
    
    Addr pf_addr = stat.mem_addr + list->last->strideToPrev;
    
    

    /*
     * Issue a prefetch request if a demand miss occured,
     * and the block is not already in cache.
     */
    if (stat.miss && !in_cache(pf_addr) && !in_mshr_queue(pf_addr) && list->judge()) {
        issue_prefetch(pf_addr);
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
