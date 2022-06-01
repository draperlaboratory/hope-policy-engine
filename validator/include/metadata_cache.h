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

#ifndef METADATA_CACHE_H
#define METADATA_CACHE_H

#include <memory>
#include <unordered_map>
#include "metadata.h"

namespace policy_engine {

class metadata_cache_t {
private:
  std::unordered_map<metadata_t, std::shared_ptr<metadata_t>, metadata_t::hasher_t, metadata_t::equal_t> map;
public:
  std::shared_ptr<metadata_t> canonize(metadata_t md) {
    if (map.find(md) == map.end())
      map[md] = std::make_shared<metadata_t>(md);
    return map[md];
  }

  template<class MetadataPtr>
  std::shared_ptr<metadata_t> canonize(MetadataPtr md) { canonize(*md); }
};

} // namespace policy_engine

#endif
