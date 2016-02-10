
#include <cmath>
#include "interface.hh"
#include <vector>

static const int CONSECUTIVE_STRIDES = 3; //HOW MANY STRIDES TO COMPARE BEFORE ISSUING PREFETCH BASED ON STRIDE


bool judge();

std::vector<Addr> list;


bool judge() {
    std::vector<Addr>::iterator it = list.end();
    const int LIMIT = std::min(CONSECUTIVE_STRIDES, int(list.size()));
    std::advance(it,-LIMIT);
    for (; it != list.end(); ++it) {
        if (*it - *(it+1) == *(it+1) - *(it+2)) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

void prefetch_init(void)
{
    

}

void prefetch_access(AccessStat stat)
{
    list.push_back(stat.mem_addr);
    
    if (list.size() > CONSECUTIVE_STRIDES +1) {
        list.erase(list.begin());
    }
    
    Addr pf_addr = stat.mem_addr + (list.end() - (list.end()++));
    
    if (stat.miss && !in_cache(pf_addr) && judge()) {
        issue_prefetch(pf_addr);
    }
}

void prefetch_complete(Addr addr) {

}
