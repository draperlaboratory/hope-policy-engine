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

#ifndef METADATA_MEMORY_MAP_H
#define METADATA_MEMORY_MAP_H

#include <stdint.h>
#include <list>

#include "platform_types.h"
#include "metadata.h"
#include "metadata_cache.h"

namespace policy_engine {

  class metadata_memory_map_t {

    struct range_t { address_t start, end; };    
    
    struct mem_tag_boundary_t {
      address_t start;            // boundary 
      metadata_t const *metadata; // metadata to right of boundary
    };
    
  private:
    
    metadata_cache_t *md_cache;  
    std::list<mem_tag_boundary_t> mrs;
    
  public:
    
    void add_range(address_t start, address_t end, metadata_t const *metadata);
    
    metadata_t const *get_metadata(address_t addr) {
      
      mem_tag_boundary_t lmr;
      
      if ( !addr )
	return nullptr;
      
      /* todo: make binary search? */
      for ( auto &mr : mrs ) {
	if ( mr.start > addr)
	  return lmr.metadata;
	lmr = mr;
      }
    
      return nullptr;
    }
  
  metadata_memory_map_t() { md_cache = new metadata_cache_t(); }
  metadata_memory_map_t(metadata_cache_t *mc) : md_cache(mc) { }
  
    template <class Type, class UnqualifiedType = std::remove_cv<Type> >
      class ForwardIterator 
      : public std::iterator<std::forward_iterator_tag,
      UnqualifiedType,
      std::ptrdiff_t,
      Type*,
      Type&> {
    private:

    std::list<mem_tag_boundary_t>::iterator mb_it;
    std::list<mem_tag_boundary_t> &mbs;
    
    typedef std::pair<range_t, metadata_t const *> result_type_t;      
    result_type_t current;

    void advance() {

      /* skip empty regions */
      while ( (mb_it != mbs.end()) && !mb_it->metadata )
	++mb_it;

      /* dont advance if done */
      if ( mb_it == mbs.end() )
	return;

      /* record start */
      current.first.start = mb_it->start;
      current.second = mb_it->metadata;

      /* record end (ie begininning of next memory region) */
      if ( ++mb_it == mbs.end() )
	current.first.end = ~0;
      else
	current.first.end = mb_it->start;

      return;
    }

    public:

    /* constructor for iterator */
    ForwardIterator(metadata_memory_map_t *map, bool begin) : mbs(map->mrs) {
      if ( begin && (mbs.begin() != mbs.end()) ) {
	mb_it = mbs.begin();
	advance(); // set current to the 0th val
      }
      else { // bool implies end or the struct is empty
	mb_it = mbs.end();
      }
    }

    void swap(ForwardIterator& other) noexcept {
      using std::swap;
      swap(mbs, other.mbs);
      swap(mb_it, other.mb_it);
    }
    
    // Pre-increment
    ForwardIterator& operator++ () {
      advance();
      return *this;
    }
    
    // Post-increment
    ForwardIterator operator++ (int) {
      ForwardIterator tmp(*this);
      advance();
      return tmp; 
    }
    
    // two-way comparison: v.begin() == v.cbegin() and vice versa
    template<class OtherType>
    bool operator == (const ForwardIterator<OtherType>& rhs) const {
      return ((mb_it == mbs.end()) && (rhs.mb_it == rhs.mbs.end())) ||
      ((mb_it->start == rhs.mb_it->start) && (mb_it->metadata == rhs.mb_it->metadata));
       }
    
      template<class OtherType>
      bool operator != (const ForwardIterator<OtherType>& rhs) const {
	return !(this->operator == (rhs));
      }
      result_type_t &operator* () { return current; }
      result_type_t *operator-> () { return &current; }
    };
  
    typedef ForwardIterator<metadata_memory_map_t> iterator;
    typedef ForwardIterator<const metadata_memory_map_t> const_iterator;

    iterator begin() { return iterator(this, true); }
    iterator end() { return iterator(this, false); }
    };

  } // namespace policy_engine

#endif
