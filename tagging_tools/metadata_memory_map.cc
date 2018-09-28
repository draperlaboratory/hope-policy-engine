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
  
  /* this is a meaningless call */
  if ( start >= end )
    return;

  /* initialize empty memory region list */
  if ( mrs.size() == 0 ) {
    mem_tag_boundary_t zero_mb;
    zero_mb.start = 0;
    zero_mb.metadata = NULL;
    mrs.push_back(zero_mb);
  }
  
  mem_tag_boundary_t new_mb;
  new_mb.start = start;
  new_mb.metadata = md_cache->canonize(metadata);
  
  mem_tag_boundary_t end_mb;
  end_mb.start = end;
  
  iterator lmb, hmb, nmb;
  
  /* find next memory boundary after start of new region */
  for ( hmb = mrs.begin(); hmb != mrs.end(); ++hmb ) {
    if ( hmb.start >= start )
      break;
    lmb = hmb;
  }
    
  /* add a new memory boundary for start of new region */
  if ( hmb.start != start )
    nmb = mrs.insert(hmb, new_mb);
  else
    nmb = hmb;
  
  /* inherit meteadata of left boundary */
  nmb->add_md(lmb);

  /* until end */
  for ( ; hmb != mrs.end(); ++hmb ) {
    if ( hmb.start >= end )
      break;
    lmb = hmb;

    /* inherit new metadata */
    hmb->add_md(nmb);
  }

  if ( (hmb == mrs.end()) || (hmb.start == end))
    return;

  /* add end boundary */
  end_mb.metadata = lmb.metadata;
  mrs.insert(hmb, end_mb);

  return;
}

