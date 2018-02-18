#ifndef METADATA_MEMORY_MAP_H
#define METADATA_MEMORY_MAP_H

#include <stdint.h>
#include <vector>

#include "platform_types.h"
#include "metadata.h"
#include "metadata_cache.h"

namespace policy_engine {

class metadata_memory_map_t {
  address_t base;
  address_t end_address;
  metadata_cache_t *md_cache;
  
  static const int stride = sizeof(uint32_t); // platform word size
  std::vector<metadata_t const *> map;

  protected:
  address_t index_to_addr(size_t idx) {
    return base + (idx * stride);
  }

  size_t addr_to_index(address_t addr) {
    return (addr - base) / stride;
  }

  public:

  struct range_t { address_t start, end; };

  void add_range(address_t start, address_t end, metadata_t const *metadata);
  metadata_t const *get_metadata(address_t addr) {
    if ((addr >= base) && (addr < end_address))
      return map[addr_to_index(addr)];
    return nullptr;
  }

  metadata_memory_map_t(address_t base, metadata_cache_t *mc) : base(base), md_cache(mc) { }

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
  
  typedef ForwardIterator<metadata_memory_map_t> iterator;
  typedef ForwardIterator<const metadata_memory_map_t> const_iterator;

  iterator begin() { return iterator(this); }
  iterator end() { return iterator(this, true); }
};

} // namespace policy_engine

#endif
