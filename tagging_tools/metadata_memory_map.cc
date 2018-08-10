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
  
void metadata_memory_map_t::add_range(address_t start, address_t end, metadata_t *metadata) {

  /* find the right mr */
  for ( auto &mr : mrs ) {

    /* check whether the region is adjacent or contained within */
    /* TODO: if the overhead of an entire other vector is large, we can 
       add a "slop" figure here so that two ranges that are close to 
       each other use the same vector even if not exactly adjacent */
    if ( ((start <= mr.end) && (start >= mr.base)) ||
	 ((end >= mr.base)  && (end <= mr.end))    ) {
      mr.add_range(start, end, metadata);
      return;
    }
  }

  if ( start == end )
    return;
  
  /* an appropriate existing MR was not found - make a new one */
  mem_region_t mr = mem_region_t();
  int len = mrs.size();

  /* put it in the vector at the right location */
  int i;
  for ( i = 0; i < len; i++ ) {
    if ( end < mrs[i].base )
      mrs.insert(mrs.begin()+i, mr);
  }
  if ( (i == len) ) // at the end
    mrs.push_back(mr);

  /* now add the metadata range */
  mrs[len].add_range(start, end, metadata);

  return;
}

