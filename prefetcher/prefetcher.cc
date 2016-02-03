/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "interface.hh"
#include "stdint.h"

#define N 3

Addr* last_addrs;
uint8_t head;


void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");

    last_addrs = (Addr*)malloc(sizeof(Addr) * N);
    head = 0;
}

void prefetch_access(AccessStat stat)
{
    last_addrs[head++] = stat.mem_addr;
    if (head == N) {
        head = 0;
    }

    bool pattern = true;
    int32_t stride = last_addrs[1] - last_addrs[0];
    for (uint8_t i = 2; i < N; i++) {
        if (last_addrs[i] - last_addrs[i-1] != stride) {
            pattern = false;
            break;
        }
    }

    if (pattern) {
        Addr pf_addr = last_addrs[N-1] + stride;
        if (!in_cache(pf_addr)) {
            issue_prefetch(pf_addr);
        }
    }
}

void prefetch_complete(Addr addr) {
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */

    free(last_addrs);
}

