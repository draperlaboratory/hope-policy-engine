#ifndef METADATA_H
#define METADATA_H

#include <assert.h>
#include <stdint.h>

#include <cstdlib>
#include <iterator>
#include <unordered_set>

#include "policy_types.h"
class metadata_t {
  std::size_t hash;
  std::unordered_set<meta_t> tags;

  public:
  struct hasher_t {
    std::size_t operator()(const metadata_t &k) const {
      return k.hash;
    }
  };

  struct equal_t {
    bool operator()(metadata_t const &l, metadata_t const &r) const {
      return l.tags == r.tags;
    }
  };

  metadata_t() { }

  size_t size() const { return tags.size(); }

  bool operator ==(const metadata_t &rhs) const {
    return tags == rhs.tags;
  }

  bool operator !=(const metadata_t &rhs) const { return !(*this == rhs); }

  void insert(const meta_t &rhs) {
    hash += rhs;
    tags.insert(rhs);
  }

//  metadata_t& operator +=(const meta_t &rhs) {
//    hash += rhs;
//    tags.insert(rhs);
//    return *this;
//  }

  void insert(const metadata_t *rhs) {
    hash += rhs->hash;
    tags.insert(rhs->begin(), rhs->end());
  }

//  metadata_t& operator |=(const metadata_t &rhs) {
//    hash += rhs.hash;
//    tags.insert(rhs.begin(), rhs.end());
//    return *this;
//  }

  typedef std::unordered_set<meta_t>::iterator iterator;
  typedef std::unordered_set<meta_t>::const_iterator const_iterator;

  const_iterator begin() const { return tags.begin(); }
  const_iterator end() const { return tags.end(); }
  iterator begin() { return tags.begin(); }
  iterator end() { return tags.end(); }
};

//inline metadata_t& operator +(const metadata_t &lhs, const meta_t &rhs) {
//  return metadata_t(lhs) += rhs;
//}

//inline metadata_t& operator |(const metadata_t &lhs, const metadata_t &rhs) {
//  return metadata_t(lhs) |= rhs;
//}

#if 0
class metadata_t {
  static constexpr int MAX_WORDS = 8;
  static constexpr int MAX_TAG = MAX_WORDS * 8;
  uint32_t tags[MAX_WORDS];

  static void decompose_index(meta_t v, int &index, uint32_t &mask) {
    index = v / 32;
    int bit = v % 32;
    mask = 1 << bit;
  }
  
  bool is_set(int bit_num) const {
    int index;
    uint32_t mask;
    decompose_index(bit_num, index, mask);
    return (tags[index] & mask) != 0;
  }

  void set(int bit_num) {
    int index;
    uint32_t mask;
    decompose_index(bit_num, index, mask);
    tags[index] |= mask;
  }

  public:
  struct metadata_hasher_t {
    std::size_t operator()(const metadata_t &k) const {
      size_t hash = 0;
      for (int i = 0; i < MAX_WORDS; i++)
	hash += k.tags[i];
      return hash;
    }
  };

  struct meta_data_equal_t {
    bool operator()(metadata_t const &l, metadata_t const &r) const {
      for (int i = 0; i < MAX_WORDS; i++)
	if (l.tags[i] != r.tags[i])
	  return false;
      return true;
    }
  };

  metadata_t() { }

  void clear() {
    for (int i = 0; i < MAX_WORDS; i++)
      tags[i] = 0;
  }

  bool operator ==(const metadata_t &rhs) const {
    for (int i = 0; i < MAX_WORDS; i++) {
      if (tags[i] != rhs.tags[i])
	return false;
    }
    return true;
  }

  bool operator !=(const metadata_t &rhs) const { return !(*this == rhs); }

  metadata_t& operator +=(const meta_t &rhs) {
    if (rhs > MAX_TAG)
      throw "too many tags";
    set(rhs);
/*
    int index = rhs / 32;
    int bit = rhs % 32;
    uint32_t mask = 1 << bit;
    
    tags[index] |= mask;
*/
    return *this;
  }

  metadata_t& operator |=(const metadata_t &rhs) {
    for (int i = 0; i < MAX_WORDS; i++)
      tags[i] |= rhs.tags[i];
    return *this;
  }

  /*
    And now some iterators, so we can get all the individual meta_t items
    back out.
    */

  template <class Type, class UnqualifiedType = std::remove_cv<Type> >
    class ForwardIterator : public std::iterator<std::forward_iterator_tag,
    UnqualifiedType, std::ptrdiff_t, Type*, Type&> {
    typedef meta_t result_type_t;
    int bit_num;
    metadata_t const *data;
    result_type_t current;

    public:

    void advance() {
      if (bit_num < MAX_TAG) {
	do {
	  bit_num++;
	} while ((bit_num < MAX_TAG) && !data->is_set(bit_num));
	current = 1 << bit_num;
      }
    }

    ForwardIterator(metadata_t const *data, bool end) : data(data), bit_num(MAX_TAG) { }

    explicit ForwardIterator(metadata_t const *data) : data(data), bit_num(0) { }
    
    void swap(ForwardIterator& other) noexcept {
      using std::swap;
      swap(data, other.data);
      swap(bit_num, other.bit_num);
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
      return bit_num == rhs.bit_num;
    }
    
    template<class OtherType>
    bool operator != (const ForwardIterator<OtherType>& rhs) const {
        return bit_num != rhs.bit_num;
    }
    result_type_t &operator* () { return current; }
    result_type_t *operator-> () { return &current; }
    
    // One way conversion: iterator -> const_iterator
    operator ForwardIterator<const Type>() const {
      return ForwardIterator<const Type>(data);
    }
  };
  
  typedef ForwardIterator<metadata_t> iterator;
  typedef ForwardIterator<const metadata_t> const_iterator;

  const_iterator begin() const { return const_iterator(this); }
  const_iterator end() const { return const_iterator(this, true); }

  void print() const;
  
};

inline metadata_t& operator +(const metadata_t &lhs, const meta_t &rhs) {
  return metadata_t(lhs) += rhs;
}

inline metadata_t& operator |(const metadata_t &lhs, const metadata_t &rhs) {
  return metadata_t(lhs) |= rhs;
}
#endif
#endif
