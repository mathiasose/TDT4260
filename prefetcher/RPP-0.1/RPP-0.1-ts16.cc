/* ----------------------------------------------------------------------------
 * Reference Prediction Prefetcher (RPP)
 * ----------------------------------------------------------------------------
 * TODO: Expand this comment.
 *
 * ----------------------------------------------------------------------------
 * Description
 * ----------------------------------------------------------------------------
 * The prefetcher uses a Reference Prediction Table (RPT):
 *     +------------------------------------------+
 *     |  Tag  |  Prev_addr  |  Stride  |  State  |
 *     +------------------------------------------+
 * 
 * TODO: Expand this comment.
 *
 */

#include "interface.hh"

#define TABLE_SIZE 16 


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
