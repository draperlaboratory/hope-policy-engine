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

#include <cstdint>
#include <memory>
#include <vector>
#include "metadata.h"
#include "metadata_cache.h"
#include "metadata_factory.h"
#include "reporter.h"

namespace policy_engine {
  
struct range_t {
  uint64_t start, end;
  bool operator <(const range_t& other) const { return (start < other.start) || (start == other.start && end < other.end); }
};

class metadata_memory_map_t {
private:
  class mem_region_t {
    friend class metadata_memory_map_t;

  private:
    range_t range;
    std::vector<std::shared_ptr<metadata_t>> mem;
    metadata_memory_map_t* map; // must be a raw pointer so it doesn't get cleaned up when mem_region_t does

  public:
    /* expose iterator of inner vector */
    using iterator = typename decltype(mem)::iterator;
    using const_iterator = typename decltype(mem)::const_iterator;

    static const int stride = sizeof(uint32_t); // platform word size

    mem_region_t(metadata_memory_map_t& m) : range({0, 0}), map(&m) {}
    
    iterator begin() { return mem.begin(); }
    iterator end() { return mem.end(); }
    const_iterator begin() const noexcept { return mem.cbegin(); }
    const_iterator end() const noexcept { return mem.cend(); }
    const_iterator cbegin() const noexcept { return mem.cbegin(); }
    const_iterator cend() const noexcept { return mem.cend(); }

    uint64_t itr_to_addr(iterator itr) const { return index_to_addr(itr - begin()); }
    uint64_t itr_to_addr(const_iterator itr) const { return index_to_addr(itr - cbegin()); }
    uint64_t index_to_addr(size_t idx) const { return range.start + (idx*stride); }
    size_t addr_to_index(uint64_t addr) const { return (addr - range.start)/stride; }
    std::shared_ptr<metadata_t> getaddr(uint64_t addr) const { return mem[addr_to_index(addr)]; }
    bool contains(uint64_t addr) const { return (addr >= range.start) && (addr <= range.end); }
    size_t size() const { return mem.size(); }
    
    void add_range(uint64_t start, uint64_t end, std::shared_ptr<metadata_t> metadata) {
      if (range.start == range.end) {
        range.start = start;
        assert(mem.size() == 0); // first range added
      }
      else if (start < range.start) {
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
  };

  uint64_t base;
  uint64_t end_address;
  metadata_cache_t md_cache;
  std::vector<mem_region_t> mrs;

public:
  void add_range(uint64_t start, uint64_t end, std::shared_ptr<metadata_t> metadata);

  std::shared_ptr<metadata_t> get_metadata(uint64_t addr) {
    for (auto& mr : mrs) {
      if (mr.contains(addr))
        return mr.getaddr(addr);
    }
    return nullptr;
  }

  metadata_memory_map_t() : base(-1) {}

  template <class MMap, class MRIterator, class MRVIterator>
  class ForwardIterator {
  private:
    using result_type_t = std::pair<range_t, std::shared_ptr<metadata_t>>;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::remove_cv<MMap>;
    using difference_type = std::ptrdiff_t;
    using pointer = const result_type_t*;
    using reference = const result_type_t&;

  private:
    MMap* map;
    MRIterator mr_it;
    MRVIterator it;
    result_type_t current;

    void advance() {
      /* don't advance if done */
      if (it == map->mrs.end()) {
        mr_it = mem_region_t::iterator(nullptr);
        return;
      }

      /* move to next mr if necessary */
      if (mr_it == it->end()) {
        if (++it != map->mrs.end()) 
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
      } while (((mr_it) != it->end()) && (*mr_it == *(mr_it - 1)));

      /* record end */
      if (mr_it == it->end())
        current.first.end = it->range.end;
      else
        current.first.end = it->itr_to_addr(mr_it);
    }
  
  public:
    /* constructor for iterator */
    ForwardIterator(MMap* map, bool begin) : map(map) {
      if (begin && (map->mrs.begin() != map->mrs.end())) {
        it = map->mrs.begin();
        mr_it = it->begin();
        advance(); // set current to the 0th val
      } else { // bool implies end or the struct is empty
        it = map->mrs.end();
        mr_it = mem_region_t::iterator(nullptr);
      }
    }
  
    // Pre-increment
    ForwardIterator& operator ++() {
      advance();
      return *this;
    }
  
    // Post-increment
    ForwardIterator operator ++(int) {
      ForwardIterator tmp(*this);
      advance();
      return tmp; 
    }
  
    // two-way comparison: v.begin() == v.cbegin() and vice versa
    template<class OtherMMap, class OtherMRIterator, class OtherMRVIterator>
    bool operator ==(const ForwardIterator<OtherMMap, OtherMRIterator, OtherMRVIterator>& rhs) const {
      return ((it == map->mrs.end()) && (rhs.it == rhs.map->mrs.end())) || (((mr_it - it->begin()) == (rhs.mr_it - rhs.it->begin())) && (it == rhs.it));
    }
    template<class OtherMMap, class OtherMRIterator, class OtherMRVIterator>
    bool operator !=(const ForwardIterator<OtherMMap, OtherMRIterator, OtherMRVIterator>& rhs) const {
      return !(*this == rhs);
    }
  
    reference operator *() { return current; }
    pointer operator ->() { return &current; }
  };

  using iterator = ForwardIterator<metadata_memory_map_t, mem_region_t::iterator, std::vector<mem_region_t>::iterator>;
  using const_iterator = ForwardIterator<const metadata_memory_map_t, mem_region_t::const_iterator, std::vector<mem_region_t>::const_iterator>;

  iterator begin() { return iterator(this, true); }
  iterator end() { return iterator(this, false); }
  const_iterator begin() const noexcept { return const_iterator(this, true); }
  const_iterator end() const noexcept { return const_iterator(this, false); }
  const_iterator cbegin() const noexcept { return const_iterator(this, true); }
  const_iterator cend() const noexcept { return const_iterator(this, false); }
};

} // namespace policy_engine

#endif
