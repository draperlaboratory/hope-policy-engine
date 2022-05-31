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

#include <memory>
#include <string.h>
#include <unordered_map>
#include "policy_meta_set.h"
#include "metadata.h"

namespace policy_engine {

class meta_set_cache_t {
  public:
  struct meta_set_hasher_t {
    std::size_t operator()(const meta_set_t &k) const {
      size_t hash = 0;
      for (int i = 0; i < META_SET_WORDS; i++)
	hash += k.tags[i];
      return hash;
    }
  };
  struct meta_set_equal_t {
    bool operator()(meta_set_t const &l, meta_set_t const &r) const {
      for (int i = 0; i < META_SET_WORDS; i++)
	if (l.tags[i] != r.tags[i])
	  return false;
      return true;
    }
  };
  private:
  std::unordered_map<meta_set_t, meta_set_t *, meta_set_hasher_t, meta_set_equal_t> map;
  public:
  meta_set_t const *canonize(meta_set_t const &ts) {
    if (map.find(ts) == map.end()) {
      meta_set_t *ms = new meta_set_t();
      *ms = ts;
      map[ts] = ms;
    }
    return map[ts];
  }
  meta_set_t const *canonize(metadata_t const *metadata) {
    meta_set_t ms;
    memset(&ms, 0, sizeof(ms));
    for (auto e: *metadata)
      ms_bit_add(&ms, e);
    return canonize(ms);
  }
  const meta_set_t* canonize(std::shared_ptr<const metadata_t> metadata) {
    return canonize(metadata.get());
  }
};

} // namespace policy_engine

#endif
