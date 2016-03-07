/* ----------------------------------------------------------------------------
 * Simple Stride Directed Prefetcher (SSDP)
 * ----------------------------------------------------------------------------
 * A prefetcher based on the approach "Stride Directed Prefetching (SDP)"
 * as described in the doctoral thesis:
 *     Reducing Memory Latency by Improving Resource Utilization
 *     by Marius Grannæs
 *     http://www.idi.ntnu.no/research/doctor_theses/grannas.pdf
 *
 * ----------------------------------------------------------------------------
 * Description
 * ----------------------------------------------------------------------------
 * When the prefetcher receives a load instruction, it uses the instruction
 * address as an index into a "reference table":
 *     +----------------+
 *     |  Last address  |
 *     +----------------+
 *
 * The reference table contains the memory address previously referenced by the
 * instruction. The old memory address is subtracted from the new one to
 * generate a stride, and that stride is added to the new address to generate
 * the prefetch address.
 *
 * NOTE: Many load instructions map to the same table entry and will replace
 * each other. When a load instruction is replaced, an invalid stride will be
 * generated because of the assumption that the current load instruction always
 * owns the corresponding entry.
 */

#include "interface.hh"

#define TABLE_SIZE 128


// A table entry.
struct LoadInstruction {
    Addr prev_addr;
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
        table[i].prev_addr = NULL;
    }
}


// Return true if the table contains the load instruction.
bool ReferenceTable::has(Addr pc) {
    int i = pc % TABLE_SIZE;
    return (table[i].prev_addr != NULL);
}


// Create a new entry in the reference table.
void ReferenceTable::add(Addr pc, Addr prev_addr) {
    int i = pc % TABLE_SIZE;
    table[i].prev_addr = prev_addr;
}


// Return the entry matching the specified address.
LoadInstruction * ReferenceTable::get(Addr pc) {
    int i = pc % TABLE_SIZE;
    return &table[i];
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
