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

#ifndef METADATA_H
#define METADATA_H

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include "policy_meta_set.h"
#include "policy_types.h"

namespace policy_engine {

struct metadata_t {
  std::size_t hash = 0;
  std::set<meta_t> tags;

  size_t size() const { return tags.size(); }

  bool operator ==(const metadata_t &rhs) const { return tags == rhs.tags; }
  bool operator !=(const metadata_t &rhs) const { return !(*this == rhs); }

  void insert(const meta_t &rhs) {
    hash += rhs;
    tags.insert(rhs);
  }

  template<class MetadataPtr> void insert(const MetadataPtr rhs) {
    hash += rhs->hash;
    tags.insert(rhs->begin(), rhs->end());
  }

  typedef std::set<meta_t>::iterator iterator;
  typedef std::set<meta_t>::const_iterator const_iterator;

  const_iterator begin() const { return tags.begin(); }
  const_iterator end() const { return tags.end(); }
  iterator begin() { return tags.begin(); }
  iterator end() { return tags.end(); }
};

} // namespace policy_engine

namespace std {

template<>
struct hash<policy_engine::metadata_t> {
  std::size_t operator ()(const policy_engine::metadata_t& k) const { return k.hash; }
};

template<>
struct equal_to<policy_engine::metadata_t> {
  bool operator ()(const policy_engine::metadata_t& lhs, const policy_engine::metadata_t& rhs) const { return lhs.tags == rhs.tags; }
};

} // namespace std

#endif
