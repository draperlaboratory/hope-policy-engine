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
#include "metadata_cache.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "tag_file.h"

namespace policy_engine {

/**
   Metadata utilities commonly have to set up the same basic sets of
   objects in order to reach some basic level of function.  This class
   aggregates a baseline set of required objects, and sets them up for
   use by tools that want to create/update metadata.
*/
class metadata_tool_t {
//  reporter_t err;
  metadata_cache_t md_cache;
  metadata_factory_t md_factory;
  metadata_memory_map_t md_map;
  public:
  metadata_tool_t(const char *policy_dir) //, reporter_t err)
    : md_factory(metadata_factory_t(policy_dir)), md_map(&md_cache) {}
  metadata_t const *lookup_metadata(std::string const &dotted_path) {
    return md_factory.lookup_metadata(dotted_path);
  }
  bool apply_group_tag(uint64_t start, uint64_t end, const char *group,
                       int32_t flags, uint32_t rs1, uint32_t rs2, uint32_t rs3,
                       uint32_t rd, int32_t imm) {
    metadata_t const *metadata = md_factory.lookup_group_metadata(group, flags, rs1, rs2, rs3, rd, imm);
    if (!metadata)
      return false;
    md_map.add_range(start, end, metadata);
    return true;
  }
  bool apply_tag(uint64_t start, uint64_t end, const char *tag_name) {
    metadata_t const *md = md_factory.lookup_metadata(tag_name);
    if (!md)
      return false;
    md_map.add_range(start, end, md);
    return true;
  }
  void apply_tag(uint64_t start, uint64_t end, metadata_t const *metadata) {
    md_map.add_range(start, end, metadata);
  }
  bool load_tag_info(const char *tag_file_name) {
    return load_tags(md_map, tag_file_name);
  }
  bool save_tag_info(const char *tag_file_name) {
    return save_tags(md_map, tag_file_name);
  }
  metadata_factory_t& factory() { return md_factory; }
};

} // namespace policy_engine

#endif
