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

#include "metadata_memory_map.h"

#include "metadata_factory.h"

using namespace policy_engine;

// for debugging only
static metadata_factory_t *factory;

void init_metadata_renderer(metadata_factory_t *md_factory) {
  factory = md_factory;
}

std::string render_metadata(metadata_t const *metadata) {
  if (factory)
    return factory->render(metadata);
  return "<no renderer>";
}

void metadata_memory_map_t::add_range(address_t start, address_t end, metadata_t const *metadata) {
  if (base == -1) {
    base = start;
    assert(map.size() == 0); // first range added
  } else if (start < base) {
    // inserting before the existing base - have to insert a bit
    int n_insert = base - start;
    map.insert(map.begin(), n_insert, nullptr);
    base = start;
  }
  int s = (start - base) / stride;
  int e = (end - base) / stride;
  if (e > map.size()) {
    map.resize(e, nullptr);
    end_address = index_to_addr(e);
  }
  while (s < e) {
//      printf("0x%x, 0x%x\n", s, e);
    metadata_t const *md;
    if (map[s]) {
      md = map[s];
      metadata_t *new_md = new metadata_t(*md);
      new_md->insert(metadata);
      md = md_cache->canonize(new_md);
      if (md != new_md)
	delete new_md;
    } else {
      md = md_cache->canonize(metadata);
    }
//    std::string md_s = render_metadata(md);
//    printf("0x%08x: %s\n", s, md_s.c_str());
    map[s] = md;
    s++;
  }
}
