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

#include <string>
#include "metadata_memory_map.h"
#include "metadata_factory.h"
#include "validator_exception.h"

namespace policy_engine {

static metadata_factory_t *factory;

void init_metadata_renderer(metadata_factory_t *md_factory) {
  factory = md_factory;
}

metadata_factory_t* init(const std::string& policy_dir) {
  try {
    metadata_factory_t* md_factory = new metadata_factory_t(policy_dir);
    init_metadata_renderer(md_factory);
    return md_factory;
  } catch (exception_t &e) {
    std::printf("exception: %s\n", e.what());
  }
}

std::string render_metadata(metadata_t const *metadata) {
  if (factory != nullptr)
    return factory->render(metadata);
  return "<no renderer>";
}
  
void metadata_memory_map_t::add_range(uint64_t start, uint64_t end, metadata_t const *metadata) {
  /* this is a meaningless call */
  if (start >= end)
    return;
  
  /* find the right mr */
  for (auto &mr : mrs) {

    /* check whether the region is adjacent or contained within */
    if (mr.contains(start) || mr.contains(end)) {
      mr.add_range(start, end, metadata);
      return;
    }
  }
  
  /* an appropriate existing MR was not found - make a new one */
  mem_region_t mr = mem_region_t(this);
  int len = mrs.size();

  /* put it in the vector at the right location */
  int i;
  for (i = 0; i < len; i++) {
    if (end < mrs[i].range.start) {
      /* insert in the correct place */
      mrs.insert(mrs.begin()+i, mr);

      /* now add the metadata range */
      mrs[i].add_range(start, end, metadata);
      return;
    }
  }

  /* nothing found, put it at the end. */
  mrs.push_back(mr);
  mrs[i].add_range(start, end, metadata);
   
  return;
}

}