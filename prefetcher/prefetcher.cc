/* ----------------------------------------------------------------------------
 * Stride Directed Prefetcher
 * ----------------------------------------------------------------------------
 *
 * A prefetcher based on the approach "Stride Directed Prefetching (SDP)"
 * as described in the doctoral thesis:
 *     Reducing Memory Latency by Improving Resource Utilization
 *     by Marius Grannæs
 *     http://www.idi.ntnu.no/research/doctor_theses/grannas.pdf
 *
 * ----------------------------------------------------------------------------
 * Description
 * ----------------------------------------------------------------------------
 * In SDP, the prefetcher stores load instructions into a table. Each table
 * entry contains the following:
 *     +---------------------------------------------+
 *     |  PC address  |  Last address  |  Valid bit  |
 *     +---------------------------------------------+
 *
 * PC address is the address of the load instruction. Last address is the last
 * address referenced by the load instruction. Valid bit indicates whether the
 * entry is still valid.
 *
 * The first time a load instruction is encountered, it is simplt stored in the
 * table along with the referenced memory address.
 *
 * When an instruction is encountered that already resides in the table, a
 * stride is computed from the new and old referenced memory addresses. Then, a
 * prefetch request is issued for the data located at (new address + stride).
 *
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
