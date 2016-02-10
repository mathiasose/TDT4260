/*
 * A prefetcher based on the approach "Stride Directed Prefetching (SDP)"
 * described in the doctoral thesis:
 *     Reducing Memory Latency by Improving Resource Utilization
 *     by Marius Grannæs
 *     http://www.idi.ntnu.no/research/doctor_theses/grannas.pdf
 */

#include "interface.hh"


void prefetch_init(void)
{
    DPRINTF(HWPrefetch, "Initialized stride-directed prefetcher\n");

	// TODO: Initialize prefetcher.
}

void prefetch_access(AccessStat stat)
{
	// TODO: Implement.
}

void prefetch_complete(Addr addr) {
	// TODO: Implement.
}
