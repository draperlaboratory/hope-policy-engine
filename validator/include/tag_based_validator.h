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

#ifndef TAG_BASED_VALIDATOR_H
#define TAG_BASED_VALIDATOR_H

#include <string>
#include "meta_cache.h"
#include "meta_set_factory.h"
#include "policy_meta_set.h"
#include "sim_validator.h"
#include "tag_utils.h"

namespace policy_engine {

class tag_based_validator_t : public abstract_validator_t {
protected:
  meta_set_cache_t ms_cache;
  meta_set_factory_t ms_factory;
  
public:
  tag_based_validator_t(const std::string& policy_dir) : ms_factory(meta_set_factory_t(&ms_cache, policy_dir)) {}
  virtual ~tag_based_validator_t() {}

  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual bool commit() = 0;

  // Provides the tag for a given address.  Used for debugging.
  virtual tag_t& get_tag(address_t addr) = 0;
  virtual const meta_set_t* get_meta_set(address_t addr) = 0;
};

} // namespace policy_engine

#endif
