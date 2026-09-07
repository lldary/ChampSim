#include "my_prefetcher.h"

#include <algorithm>

#include "cache.h"



uint32_t my_prefetcher::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                                uint32_t metadata_in)
{
  // TODO: LAB 3 PART 3
}

uint32_t my_prefetcher::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, uint32_t metadata_in)
{
  // TODO: LAB 3 PART 3
}
