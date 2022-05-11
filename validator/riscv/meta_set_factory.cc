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

#include <string.h>
#include "meta_set_factory.h"
#include "policy_meta_set.h"
#include "policy_utils.h"

using namespace policy_engine;

meta_set_factory_t::meta_set_factory_t(meta_set_cache_t *ms_cache, std::string policy_dir)
  : metadata_factory_t(policy_dir), ms_cache(ms_cache) {
}

meta_set_t const *meta_set_factory_t::get_meta_set(std::string dotted_path) {
  metadata_t const *metadata = lookup_metadata(dotted_path);
  if (metadata) {
    meta_set_t ms;
    memset(&ms, 0, sizeof(ms));
//    printf("get_meta_set: %s = ", dotted_path.c_str());
    for (const meta_t& m: *metadata) {
      ms_bit_add(&ms, m);
//      printf("0x%lx ", it.second);
    }
//    printf("\n");
    return ms_cache->canonize(ms);
  } else {
    return nullptr;
  }
}
