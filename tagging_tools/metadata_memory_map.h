#ifndef METADATA_MEMORY_MAP_H
#define METADATA_MEMORY_MAP_H

#include <stdint.h>
#include <vector>

#include "platform_types.h"
#include "meta_cache.h"

class metadata_memory_map_t {
  address_t base;
  address_t end_address;
  meta_set_cache_t *ms_cache;
  
  static const int stride = sizeof(uint32_t); // platform word size
  std::vector<meta_set_t *> map;

  protected:
  address_t index_to_addr(size_t idx) {
    return base + (idx * stride);
  }

  size_t addr_to_index(address_t addr) {
    return (addr - base) / stride;
  }

  public:
  struct range_t { address_t start, end; };
  void add_range(address_t start, address_t end, meta_set_t *meta_set) {
    int s = (start - base) / stride;
    int e = (end - base) / stride;
    if (e > map.size()) {
      map.resize(e, nullptr);
      end_address = index_to_addr(e);
    }
    while (s < e) {
//      printf("0x%x, 0x%x\n", s, e);
      meta_set_t ms;
      if (map[s]) {
	ms = *map[s];
	ms_union(&ms, meta_set);
      } else {
	ms = *meta_set;
      }
      map[s] = ms_cache->canonize(ms);
      s++;
    }
  }

  metadata_memory_map_t(address_t base, meta_set_cache_t *mc) : base(base), ms_cache(mc) { }

  template <class Type, class UnqualifiedType = std::remove_cv<Type> >
    class ForwardIterator 
    : public std::iterator<std::forward_iterator_tag,
    UnqualifiedType,
    std::ptrdiff_t,
    Type*,
    Type&> {
    typedef std::pair<range_t, meta_set_t *> result_type_t;
    int cur_index;
    int end_index;
    metadata_memory_map_t *map;
    result_type_t current;

  public:
    void advance() {
      if (!is_end()) {
	if (cur_index == end_index) {
	  make_end();
	} else {
	  while (cur_index < end_index && map->map[cur_index] == nullptr) {
	    cur_index++;
	  }
	  if (cur_index < end_index) {
	    current.first.start = map->index_to_addr(cur_index);
	    current.second = map->map[cur_index];
	    while (cur_index < end_index && (map->map[cur_index] != nullptr) &&
//		   (*map->map[cur_index] == *current.second)) {
		   (map->map[cur_index] == current.second)) {
	      cur_index++;
	    }
	    current.first.end = map->index_to_addr(cur_index);
	  } else {
	    make_end();
	  }
	}
      }
    }

    ForwardIterator(metadata_memory_map_t *map, bool end) : map(map) {
      end_index = map->addr_to_index(map->end_address);
      make_end();
    }

    bool is_end() { return cur_index == end_index + 1; }
    void make_end() { cur_index = end_index + 1; }

    explicit ForwardIterator(metadata_memory_map_t *map) : map(map) {
      cur_index = 0;
      end_index = map->addr_to_index(map->end_address);
      advance();
    }
    
    void swap(ForwardIterator& other) noexcept {
      using std::swap;
      swap(map, other.map);
      swap(cur_index, other.cur_index);
      swap(end_index, other.end_index);
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
      return cur_index == rhs.cur_index;
    }
    
    template<class OtherType>
    bool operator != (const ForwardIterator<OtherType>& rhs) const {
        return cur_index != rhs.cur_index;
    }
    result_type_t &operator* () { return current; }
    result_type_t *operator-> () { return &current; }
    
    // One way conversion: iterator -> const_iterator
    operator ForwardIterator<const Type>() const {
      return ForwardIterator<const Type>(map);
    }
  };
  
//  typedef ForwardIterator<tag_collection_t> iterator;
//  typedef ForwardIterator<const tag_collection_t> const_iterator;
  typedef ForwardIterator<metadata_memory_map_t> iterator;
  typedef ForwardIterator<const metadata_memory_map_t> const_iterator;

  iterator begin() { return iterator(this); }
  iterator end() { return iterator(this, true); }
};

//it->first = range
//it->second = meta_set_t *

#endif
