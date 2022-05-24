/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <algorithm>
#include <cstdint>
#include <string>
#include "metadata_memory_map.h"
#include "range.h"

namespace policy_engine {
  
void metadata_memory_map_t::mem_region_t::add_range(uint64_t start, uint64_t end, std::shared_ptr<metadata_t> metadata) {
  if (range.start == range.end) {
    range.start = start;
    assert(mem.size() == 0); // first range added
  } else if (start < range.start) {
    // inserting before the existing base - have to insert a bit
    int n_insert = (range.start - start)/stride;
    mem.insert(mem.begin(), n_insert, nullptr);
    range.start = start;
  }

  int s = (start - range.start)/stride;
  int e = (end - range.start)/stride;

  if (e > mem.size()) {
    mem.resize(e, nullptr);
    range.end = index_to_addr(e);
  }

  while (s < e) {
    if (mem[s])
      metadata->insert(mem[s]);
    mem[s++] = map->md_cache.canonize(metadata);
  }
}

void metadata_memory_map_t::add_range(uint64_t start, uint64_t end, std::shared_ptr<metadata_t> metadata) {
  /* this is a meaningless call */
  if (start >= end)
    return;

  if (auto it = std::find_if(mrs.begin(), mrs.end(), [&](const mem_region_t& r){ return r.contains(start) || r.contains(end); }); it != mrs.end()) {
    it->add_range(start, end, metadata);
  } else {
    /* an appropriate existing MR was not found - make a new one */
    mem_region_t mr = mem_region_t(*this);
    mr.add_range(start, end, metadata);
    mrs.insert(std::find_if(mrs.begin(), mrs.end(), [&](const mem_region_t& r){ return end < r.range.start; }), mr); // will add at end if not found
  }
}

std::shared_ptr<metadata_t> metadata_memory_map_t::get_metadata(uint64_t addr) const {
  for (const auto& mr : mrs) {
    if (mr.contains(addr))
      return mr.getaddr(addr);
  }
  return nullptr;
}

}