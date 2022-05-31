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

#ifndef TAG_CONVERTER_H
#define TAG_CONVERTER_H

#include <memory>
#include "platform_types.h"
#include "policy_meta_set.h"
#include "tag_utils.h"

namespace policy_engine {

/**
   The rule caches in tag based validators deal in tags.  The policy evaluation function
   deals in meta_set_t structures.  While many implementations may have a one to one mapping
   of tag to meta_set_t, it is possible that there will be a conversion.  This interface
   preserves that possibility, or at least indicates the points at which that conversion
   would need to be supported in code at the interface boundary between the policy
   evaluation function(s) and the rest of the world.
*/
struct tag_converter_t {
  virtual meta_set_t const *t_to_m(tag_t t) { return (meta_set_t const *)t; }
  virtual tag_t m_to_t(meta_set_t const *ms) { return (tag_t)ms; }
};

} // namespace policy_engine

#endif
