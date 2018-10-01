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
  
  auto lmb = mrs.begin();
  auto hmb = mrs.begin();
  
  /* find next memory boundary after start of new region */
  for ( hmb = mrs.begin(); (hmb->start <= start) && (hmb != mrs.end()); ) {
    /* TODO: find a way to avoid adding this redundant entry rather than just 
       deleting it after the fact */
    /* if we see a redundant entry, we can delete it */
    if ( (hmb != mrs.begin()) && (lmb->metadata == hmb->metadata) )
      hmb = mrs.erase(hmb);
    else
      lmb = hmb++;
  }

  /* 
     hmb = first region with start >  start
     lmb = last region with start  <= start
   */

  /* save the metadata that will exist after our region */
  end_mb.metadata = lmb->metadata;
  
  /* new range inherits meteadata of left boundary */
  if ( lmb->metadata && (lmb->metadata != new_mb.metadata) ) {
    metadata_t md = *lmb->metadata;
    
    md.insert(metadata);
    
    new_mb.metadata = md_cache->canonize(&md);
  }
  
  /* insert new boundary if we dont already have one & metadata is different */
  if ( (start != lmb->start) && (lmb->metadata != new_mb.metadata) ) 
    mrs.insert(hmb, new_mb);
    
  /* allow the loop to process the first region in special case */
  if ( lmb->start == start )
    hmb = lmb;
  
  for ( ; (hmb->start < end) && (hmb != mrs.end()); ++hmb ) {

    /* update saved ending metadata */
    end_mb.metadata = lmb->metadata;
    lmb = hmb;
    
    /* inherit new metadata */
    metadata_t md = *metadata;
    
    if ( hmb->metadata ) 
      md.insert(hmb->metadata);
    
    hmb->metadata = md_cache->canonize(&md);
  }

  /* add end boundary */
  if ( (hmb == mrs.end()) || (hmb->start != end) )
    mrs.insert(hmb, end_mb);

  return;
}

