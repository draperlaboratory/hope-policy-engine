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

  class metadata_memory_map_t {

    class mem_region_t {
    
      friend class metadata_memory_map_t;

      metadata_cache_t *md_cache;
    
      address_t base;
      address_t end;
      std::vector<metadata_t const *> mem;

      mem_region_t(metadata_memory_map_t *map) : base(-1){ md_cache = map->md_cache;}
    
      static const int stride = sizeof(uint32_t); // platform word size
    
      address_t index_to_addr(size_t idx) {
        return base + (idx * stride);
      }
    
      size_t addr_to_index(address_t addr) {
        return (addr - base) / stride;
      }

      metadata_t const *getaddr(address_t addr) {
        return mem[addr_to_index(addr)];
      }

      void add_range(address_t start, address_t lend, metadata_t const *metadata) {

        if (base == -1) {
          base = start;
          assert(mem.size() == 0); // first range added
        }
        else if (start < base) {
        
          // inserting before the existing base - have to insert a bit
          int n_insert = (base - start) / stride;
          mem.insert(mem.begin(), n_insert, nullptr);
          base = start;
        }
      
        int s = (start - base) / stride;
        int e = (lend - base) / stride;
      
        if (e > mem.size()) {
          mem.resize(e, nullptr);
          end = index_to_addr(e);
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

    struct range_t { address_t start, end; };

    void add_range(address_t start, address_t end, metadata_t const *metadata);
    metadata_t const *get_metadata(address_t addr) {

      for ( auto &mr : mrs ) {
        if ((addr >= mr.base) && (addr < mr.end))
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
      typedef std::pair<range_t, metadata_t const *> result_type_t;
      int cur_index;
      int end_index;
      int cur_m_idx;
      int end_m_idx;
      std::vector<mem_region_t> &mrs;
      result_type_t current;

    public:
      
      void advance() {

        /* only advance if not at end */
        if (!is_end()) {

          /* move on to next region if necessary */
          if ( cur_index == end_index) {
            cur_m_idx++;
            cur_index = 0;
            if ( cur_m_idx != end_m_idx ) 
              end_index = mrs[cur_m_idx].mem.size();
          }
        
          /* if we're @ the end, note it */
          if (cur_m_idx == end_m_idx) {
            make_end();
            return;
          }

          /* skip over null entries */
          while ((cur_index < end_index) &&
                 (mrs[cur_m_idx].mem[cur_index] == nullptr) ) {
            cur_index++;
          }
        
          current.first.start = mrs[cur_m_idx].index_to_addr(cur_index);
          current.second = mrs[cur_m_idx].mem[cur_index];
        
          /* find all similar entries within this contiguous range */
          while ( (cur_index < end_index) &&
                  (mrs[cur_m_idx].mem[cur_index] != nullptr)         &&
                  (*current.second == *mrs[cur_m_idx].mem[cur_index]) ) {
            
            cur_index++;
          }
        
          current.first.end = mrs[cur_m_idx].index_to_addr(cur_index);
      
        }
      }
    
    ForwardIterator(metadata_memory_map_t *map, bool end) : mrs(map->mrs) {
        end_m_idx = map->mrs.size();
        make_end();
      }

      bool is_end() { return ((cur_m_idx == end_m_idx + 1) && (cur_index == mrs[end_m_idx - 1].mem.size() + 1)); }
      void make_end() {
        cur_m_idx = end_m_idx + 1;
        cur_index = mrs[end_m_idx - 1].mem.size() + 1;
      }

      explicit ForwardIterator(metadata_memory_map_t *map) : mrs(map->mrs) {
        cur_index = 0;
        cur_m_idx = 0;
        end_index = mrs[cur_m_idx].mem.size();
        end_m_idx = mrs.size();
        advance();
      }
    
      void swap(ForwardIterator& other) noexcept {
        using std::swap;
        swap(mrs, other.mrs);
        swap(cur_index, other.cur_index);
        swap(cur_m_idx, other.cur_m_idx);
        swap(end_index, other.end_index);
        swap(end_m_idx, other.end_m_idx);
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
        return (cur_index == rhs.cur_index) && (cur_m_idx == rhs.cur_m_idx);
      }
    
      template<class OtherType>
      bool operator != (const ForwardIterator<OtherType>& rhs) const {
        return (cur_index != rhs.cur_index) || (cur_m_idx != rhs.cur_m_idx);;
      }
      result_type_t &operator* () { return current; }
      result_type_t *operator-> () { return &current; }
    
      // One way conversion: iterator -> const_iterator
      operator ForwardIterator<const Type>() const {
        return ForwardIterator<const Type>(mrs);
      }
    };
  
    typedef ForwardIterator<metadata_memory_map_t> iterator;
    typedef ForwardIterator<const metadata_memory_map_t> const_iterator;

    iterator begin() { return iterator(this); }
    iterator end() { return iterator(this, true); }
  };

} // namespace policy_engine

#endif
