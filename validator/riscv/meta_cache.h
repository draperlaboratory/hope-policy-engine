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

#ifndef META_CACHE_H
#define META_CACHE_H

#include <cstring>
#include <functional>
#include <memory>
#include <unordered_map>
#include "metadata.h"
#include "policy_meta_set.h"
#include "tag_utils.h"

namespace std {

template<>
struct hash<meta_set_t> {
  size_t operator ()(const meta_set_t& k) const {
    size_t hash = 0;
    for (int i = 0; i < META_SET_WORDS; i++)
      hash += k.tags[i];
    return hash;
  }
};

template<>
struct equal_to<meta_set_t> {
  bool operator ()(const meta_set_t& lhs, const meta_set_t& rhs) const {
    for (int i = 0; i < META_SET_WORDS; i++)
      if (lhs.tags[i] != rhs.tags[i])
        return false;
    return true;
  }
};

} // namespace std

namespace policy_engine {

class meta_set_cache_t {
private:
  std::unordered_map<meta_set_t, std::unique_ptr<meta_set_t>> map;

public:
  meta_set_t* canonize(const meta_set_t& ts) {
    if (map.find(ts) == map.end()) {
      map[ts] = std::make_unique<meta_set_t>();
      *map[ts] = ts;
    }
    return map[ts].get();
  }

  template<class MetaSetPtr>
  meta_set_t* canonize(MetaSetPtr metaset) {
    meta_set_t ms;
    memset(&ms, 0, sizeof(ms));
    for (const meta_t& e: *metaset)
      ms_bit_add(&ms, e);
    return canonize(ms);
  }

  meta_set_t* operator [](tag_t tag) const {
    for (const auto& [ ms, msp ] : map) {
      if (msp.get() == reinterpret_cast<meta_set_t*>(tag))
        return msp.get();
    }
    return nullptr;
  }

  tag_t to_tag(meta_set_t* msp) const { return reinterpret_cast<tag_t>(msp); }
};

} // namespace policy_engine

#endif
