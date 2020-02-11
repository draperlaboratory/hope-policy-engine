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
#include <vector>

#include "platform_types.h"
#include "metadata.h"
#include "metadata_cache.h"

namespace policy_engine {
  
  struct range_t {
    address_t start, end;
    bool operator<(range_t other) const {
      return (start < other.start) ||
             (start == other.start && end < other.end);
    }
  };
  
  class metadata_memory_map_t {

    class mem_region_t {
    
      friend class metadata_memory_map_t;

      metadata_cache_t *md_cache;
    
      range_t range;
      std::vector<metadata_t const *> mem;

    mem_region_t(metadata_memory_map_t *map) : range({0,0}){ md_cache = map->md_cache;}

      static const int stride = sizeof(uint32_t); // platform word size

      /* expose iterator of inner vector */
      using iterator = std::vector<metadata_t const *>::iterator;
      using const_iterator = std::vector<metadata_t const *>::const_iterator;
      
      iterator begin()        { return mem.begin(); }
      iterator end()          { return mem.end(); }
      const_iterator cbegin() { return mem.cbegin(); }
      const_iterator cend()   { return mem.cend(); }

      address_t itr_to_addr(iterator itr) {
	return index_to_addr(itr - begin());
      }
      
      address_t itr_to_addr(const_iterator itr) {
	return index_to_addr(itr - cbegin());
      }

      address_t index_to_addr(size_t idx) {
        return range.start + (idx * stride);
      }
    
      size_t addr_to_index(address_t addr) {
        return (addr - range.start) / stride;
      }

      metadata_t const *getaddr(address_t addr) {
        return mem[addr_to_index(addr)];
      }

      bool contains(address_t addr) {
	return (addr >= range.start) && (addr <= range.end);
      }
      
      size_t size() {
	return mem.size();
      }
      
      void add_range(address_t start, address_t end, metadata_t const *metadata) {

        if (range.start == range.end) {
          range.start = start;
          assert(mem.size() == 0); // first range added
        }
        else if (start < range.start) {
        
          // inserting before the existing base - have to insert a bit
          int n_insert = (range.start - start) / stride;
          mem.insert(mem.begin(), n_insert, nullptr);
          range.start = start;
        }
      
        int s = (start - range.start) / stride;
        int e = (end - range.start) / stride;
      
        if (e > mem.size()) {
          mem.resize(e, nullptr);
          range.end = index_to_addr(e);
        }

        metadata_t md;
        while (s < e) {

          md = *metadata;
          if ( mem[s] )
            md.insert(mem[s]);

          mem[s++] = md_cache->canonize(&md);
        }
      
        return;
      }

    };
  
    address_t base;
    address_t end_address;
    metadata_cache_t *md_cache;

    std::vector<mem_region_t> mrs;
  
  public:

    void add_range(address_t start, address_t end, metadata_t const *metadata);
    metadata_t const *get_metadata(address_t addr) {

      for ( auto &mr : mrs ) {
        if ( mr.contains(addr) )
          return mr.getaddr(addr);
      }
      return nullptr;
    }

    metadata_memory_map_t() : base(-1){ md_cache = new metadata_cache_t(); }
    metadata_memory_map_t(metadata_cache_t *mc) : base(-1), md_cache(mc) { }

    template <class Type, class UnqualifiedType = std::remove_cv<Type> >
      class ForwardIterator 
      : public std::iterator<std::forward_iterator_tag,
      UnqualifiedType,
      std::ptrdiff_t,
      Type*,
      Type&> {
    private:

    mem_region_t::iterator mr_it;
    std::vector<mem_region_t> &mrs;
    std::vector<mem_region_t>::iterator it;
    
    typedef std::pair<range_t, metadata_t const *> result_type_t;      
    result_type_t current;

      void advance() {

	/* dont advance if done */
	if ( it == mrs.end() ) {
	  mr_it = mem_region_t::iterator(nullptr);
	  return;
	}

	/* move to next mr if necessary */
	if ( mr_it == it->end() ) {
	  if ( ++it != mrs.end() ) 
	    mr_it = it->begin();
	  else { /* if we hit end */
	    mr_it = mem_region_t::iterator(nullptr);
	    return;
	  }
	}
	
	/* record start */
	current.first.start = it->itr_to_addr(mr_it);
	current.second = *mr_it;

	/* pass over all contiguous identical tags */
	do {
	  ++mr_it;
	} while ( ((mr_it) != it->end()) &&
		(*mr_it == *(mr_it - 1)) );

	/* record end */
	if ( mr_it == it->end() )
	  current.first.end = it->range.end;
	else
	  current.first.end = it->itr_to_addr(mr_it);

	return;
      }
    
    public:

    /* constructor for iterator */
    ForwardIterator(metadata_memory_map_t *map, bool begin) : mrs(map->mrs) {
	if ( begin && (mrs.begin() != mrs.end()) ) {
	  it = mrs.begin();
	  mr_it = it->begin();
	  advance(); // set current to the 0th val
	}
	else { // bool implies end or the struct is empty
	  it = mrs.end();
	  mr_it = mem_region_t::iterator(nullptr);
	}
      }

      void swap(ForwardIterator& other) noexcept {
        using std::swap;
        swap(mrs, other.mrs);
	swap(mr_it, other.mr_it);
	swap(it, other.it);
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
        return ((it == mrs.end()) && (rhs.it == rhs.mrs.end())) ||
	(((mr_it - it->begin()) == (rhs.mr_it - rhs.it->begin())) && (it == rhs.it));
      }
    
      template<class OtherType>
      bool operator != (const ForwardIterator<OtherType>& rhs) const {
        return !this->operator==(rhs);
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
