#ifndef METADATA_H
#define METADATA_H

#include <assert.h>
#include <stdint.h>

#include <cstdlib>
#include <iterator>
#include <unordered_set>

#include "policy_types.h"

namespace policy_engine {

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

  void insert(const metadata_t *rhs) {
    hash += rhs->hash;
    tags.insert(rhs->begin(), rhs->end());
  }

  typedef std::unordered_set<meta_t>::iterator iterator;
  typedef std::unordered_set<meta_t>::const_iterator const_iterator;

  const_iterator begin() const { return tags.begin(); }
  const_iterator end() const { return tags.end(); }
  iterator begin() { return tags.begin(); }
  iterator end() { return tags.end(); }
};

} // namespace policy_engine

#endif
