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

#ifndef META_SET_FACTORY_H
#define META_SET_FACTORY_H

#include <string>
#include "meta_cache.h"
#include "metadata_factory.h"
#include "policy_meta_set.h"

namespace policy_engine {

class meta_set_factory_t : public metadata_factory_t {
private:
  meta_set_cache_t* ms_cache;

public:
  meta_set_factory_t(meta_set_cache_t* ms_cache, const std::string& policy_dir) : metadata_factory_t(policy_dir), ms_cache(ms_cache) {}
  meta_set_t* get_meta_set(const std::string& dotted_path);
  meta_set_t* get_group_meta_set(const std::string& opgroup) { return nullptr; }
};

} // namespace policy_engine

#endif
