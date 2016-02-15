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
 * In SDP, the prefetcher stores load instructions into a "reference table".
 * Each table entry contains the following:
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

#define TABLE_SIZE 512


// A table entry.
struct LoadInstruction {
    Addr pc;
    Addr prev_addr;
    bool valid;
};


// The reference table.
//
// Currently implemented as a direct-mapped cache.
struct ReferenceTable {
    LoadInstruction table[TABLE_SIZE];

    ReferenceTable();
    bool has(Addr pc);
    void add(Addr pc, Addr prev_addr);
    LoadInstruction * get(Addr pc);
} reference_table;


// Initialize the reference table.
ReferenceTable::ReferenceTable() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        table[i].pc = NULL;
        table[i].prev_addr = NULL;
        table[i].valid = false;
    }
}


// Return true if the table contains the load instruction.
bool ReferenceTable::has(Addr pc) {
    int i = pc % TABLE_SIZE;
    return (table[i].pc == pc);
}


// Create a new entry in the reference table.
void ReferenceTable::add(Addr pc, Addr prev_addr) {
    int i = pc % TABLE_SIZE;
    table[i].pc = pc;
    table[i].prev_addr = prev_addr;
    table[i].valid = true;
}


// Return the entry matching the specified address.
LoadInstruction * ReferenceTable::get(Addr pc) {
    return &table[pc % TABLE_SIZE];
}


void prefetch_init(void)
{
    DPRINTF(HWPrefetch, "Initialized stride-directed prefetcher\n");
	// Not used.
}


void prefetch_access(AccessStat stat)
{
    LoadInstruction * instruction;
    int stride;
    Addr pf_addr;

    if (reference_table.has(stat.pc)) {
        
        // Compute prefetch address.
        instruction = reference_table.get(stat.pc);
        stride = stat.mem_addr - instruction->prev_addr;
        pf_addr = stat.mem_addr + stride;

        // Issue the prefetch.
        if (pf_addr <= MAX_PHYS_MEM_ADDR && !in_cache(pf_addr)) {
            issue_prefetch(pf_addr);
        }

        // Update the table entry.
        instruction->prev_addr = stat.mem_addr;
    } else {
        reference_table.add(stat.pc, stat.mem_addr);
    }
}


void prefetch_complete(Addr addr) {
	// Not used.
}
