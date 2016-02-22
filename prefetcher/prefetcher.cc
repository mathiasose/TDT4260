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
 *     Stride Directed Prefetching in Scalar Processors
 *     by Fu et al.
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
    PredictionState state;

    ReferencePrediction();
    bool predicts(Addr target);
};


ReferencePrediction::ReferencePrediction()
    : tag(0), prev_addr(0), stride(0), state(INITIAL)
{ }


bool ReferencePrediction::predicts(Addr target) {
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


void prefetch_access(AccessStat stat)
{
    ReferencePrediction * ref;
    bool correct;
    Addr pf_addr;

    // Enter new prediction into the table.
	if (!reference_table.has(stat.pc)) {
        ref = reference_table.get(stat.pc);
        ref->tag = stat.pc;
        ref->prev_addr = stat.mem_addr;
        ref->stride = 0;
        ref->state = INITIAL;
        return;
	}
    
    ref = reference_table.get(stat.pc);
    correct = ref->predicts(stat.mem_addr);

    // Compute the next state.
    if (correct) {
        switch (ref->state) {
            case INITIAL:        ref->state = STEADY;     break;
            case TRANSIENT:      ref->state = STEADY;     break;
            case STEADY:         ref->state = STEADY;     break;
            case NO_PREDICTION:  ref->state = TRANSIENT;  break;
        }
    } else {
        switch (ref->state) {
            case STEADY:         ref->state = INITIAL;        break;
            case INITIAL:        ref->state = TRANSIENT;      break;
            case TRANSIENT:      ref->state = NO_PREDICTION;  break;
            case NO_PREDICTION:  ref->state = NO_PREDICTION;  break;
        }
    }

    // Update entry.
    ref->stride = stat.mem_addr - ref->prev_addr;
    ref->prev_addr = stat.mem_addr;

    // Issue prefetch.
    pf_addr = stat.mem_addr + ref->stride;
    if (ref->state != NO_PREDICTION
            && pf_addr <= MAX_PHYS_MEM_ADDR
            && !in_cache(pf_addr)
            && !in_mshr_queue(pf_addr)) {

        issue_prefetch(pf_addr);
    }
}


void prefetch_init(void)
{
    DPRINTF(HWPrefetch, "Initialized stride-directed prefetcher\n");
	// Not used.
}


void prefetch_complete(Addr addr) {
	// Not used.
}
