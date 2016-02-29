/* ----------------------------------------------------------------------------
 * Reference Prediction Prefetcher (RPP)
 * ----------------------------------------------------------------------------
 * A prefetcher based on "Reference Prediction Tables" as described in the
 * doctoral thesis:
 *     Reducing Memory Latency by Improving Resource Utilization
 *     by Marius Grannæs
 *     http://www.idi.ntnu.no/research/doctor_theses/grannas.pdf
 *
 * A detailed description of the the technique is given in
 *     Effective Hardware-Based Data Prefetching for High-Performance
 *     Processors
 *     by Chen et al.
 *     http://www.cecs.pdx.edu/~alaa/ece587/papers/chen_ieeetoc_1995.pdf
 *
 * ----------------------------------------------------------------------------
 * Description
 * ----------------------------------------------------------------------------
 * Similar to the stride directed prefetcher, the prefetcher uses a Reference
 * Prediction Table (RPT) to store memory references. The new feature is the
 * state field.
 *     +------------------------------------------+
 *     |  Tag  |  Prev_addr  |  Stride  |  State  |
 *     +------------------------------------------+
 * 
 * There are four different states:
 *     - INITIAL
 *     - TRANSIENT
 *     - STEADY
 *     - NO_PREDICTION
 *
 * The state field is used to give a table entry history. An entry in state
 * STEADY has made two or more correct predictions recently, while an entry in
 * state NO_PREDICTION has two or more incorrect predictions recently.
 * Depending on whether the predictions are correct, the state of an entry
 * changes in the following manner:
 *     Correct:   STEADY <-- TRANSIENT <-- NO_PREDICTION
 *     Incorrect: STEADY --> TRANSIENT --> NO_PREDICTION
 *
 *     (The INITIAL state enters either STEADY or TRANSIENT)
 *
 * An entry generates a prefetch if it is not in the state NO_PREDICTION.
 */


#include "interface.hh"

#define TABLE_SIZE 128


enum PredictionState {
	INITIAL, TRANSIENT, STEADY, NO_PREDICTION,
};


struct ReferencePrediction {
    Addr tag;
    Addr prev_addr;
	uint32_t stride;
    uint32_t times;
    PredictionState state;

    ReferencePrediction();
    bool is_strided(Addr target);
};


ReferencePrediction::ReferencePrediction()
    : tag(0), prev_addr(0), stride(0), times(1), state(INITIAL)
{ }


bool ReferencePrediction::is_strided(Addr target) {
    return target == prev_addr + stride;
}


// The reference prediction table table.
//
// Currently implemented as a direct-mapped cache.
struct PredictionTable {
    ReferencePrediction table[TABLE_SIZE];

    bool has(Addr pc);
    ReferencePrediction * get(Addr pc);
} reference_table;


bool PredictionTable::has(Addr pc) {
    int index = pc % TABLE_SIZE;
    return pc == table[index].tag;
}


ReferencePrediction * PredictionTable::get(Addr pc) {
    int index = pc % TABLE_SIZE;
    return &table[index];
}


void try_prefetch(Addr addr) {
    if (!in_cache(addr)
            && !in_mshr_queue(addr) 
            && addr <= MAX_PHYS_MEM_ADDR) {
        issue_prefetch(addr);
    }
}


void prefetch_access(AccessStat stat)
{
    ReferencePrediction * ref;
    bool strided;
    Addr pf_addr;

    // Enter new prediction into the table.
	if (!reference_table.has(stat.pc)) {
        ref = reference_table.get(stat.pc);
        ref->tag = stat.pc;
        ref->prev_addr = stat.mem_addr;
        ref->stride = 0;
        ref->times = 1;
        ref->state = INITIAL;
        return;
	}
    
    ref = reference_table.get(stat.pc);
    strided = ref->is_strided(stat.mem_addr);


    if (!strided && ref->state == INITIAL) {

        ref->stride = stat.mem_addr - ref->prev_addr;
        ref->state = TRANSIENT;

    } else if (strided
            && (ref->state == INITIAL 
                || ref->state == TRANSIENT 
                || ref->state == STEADY)) {

        // Enter steady state.
        ref->state = STEADY;

    } else if (!strided && ref->state == STEADY) {

        // Exit steady state.
        //
        // Leave stride unchanged. This increases performance for strided
        // access sequences containing single unstrided accesses.
        ref->state = INITIAL;

    } else if (!strided && ref->state == TRANSIENT) {

        // Irregular pattern detected: enter NO_PREDICTION.
        ref->state = NO_PREDICTION;
        ref->times = 1;

    } else if (strided && ref->state == NO_PREDICTION) {

        // Regular pattern detected: exit NO_PREDICTION.
        ref->state = TRANSIENT;

    } else if (!strided && ref->state == NO_PREDICTION) {

        // Irregular pattern: stay in NO_PREDICTION.
        ref->stride = stat.mem_addr - ref->prev_addr;
    }

    // Issue prefetch.
    pf_addr = stat.mem_addr + ref->stride * ref->times;
    if (ref->state != NO_PREDICTION) {
        try_prefetch(pf_addr);
    }

    // If the previous prefetch has not completed yet.
    if (strided && ref->state == STEADY && !in_cache(ref->prev_addr)) {

        // Increase the lookahead and issue an extra prefetch.
        ref->times++;
        try_prefetch(stat.mem_addr + ref->stride * ref->times);
    }

    ref->prev_addr = stat.mem_addr;
}


void prefetch_init(void) {
    DPRINTF(HWPrefetch, "Initialized stride-directed prefetcher\n");
	// Not used.
}


void prefetch_complete(Addr addr) {
	// Not used.
}
