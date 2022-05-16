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

#ifndef METADATA_TOOL_H
#define METADATA_TOOL_H

#include <cstdint>
#include <memory>
#include <string>
#include "metadata.h"
#include "metadata_cache.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "riscv_isa.h"
#include "tag_file.h"

namespace policy_engine {

/**
 * Metadata utilities commonly have to set up the same basic sets of
 * objects in order to reach some basic level of function.  This class
 * aggregates a baseline set of required objects, and sets them up for
 * use by tools that want to create/update metadata.
 */
class metadata_tool_t {
private:
  metadata_memory_map_t md_map;

public:
  metadata_factory_t factory;

  metadata_tool_t(const std::string& policy_dir) : factory(metadata_factory_t(policy_dir)) {}

  bool apply_group_tag(uint64_t start, uint64_t end, const std::string& group, const decoded_instruction_t& inst) {
    std::shared_ptr<metadata_t> metadata = factory.lookup_group_metadata(group, inst);
    if (!metadata)
      return false;
    md_map.add_range(start, end, metadata);
    return true;
  }

  bool apply_tag(uint64_t start, uint64_t end, const std::string& tag_name) {
    std::shared_ptr<metadata_t> md = factory.lookup_metadata(tag_name);
    if (!md)
      return false;
    md_map.add_range(start, end, md);
    return true;
  }

  void apply_tag(uint64_t start, uint64_t end, std::shared_ptr<metadata_t> metadata) { md_map.add_range(start, end, metadata); }
  bool load_tag_info(const std::string& tag_file_name) { return load_tags(md_map, tag_file_name); }
  bool save_tag_info(const std::string& tag_file_name) { return save_tags(md_map, tag_file_name); }
};

} // namespace policy_engine

#endif
