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

#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include "metadata_cache.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "range_map.h"
#include "tag_file.h"
#include "reporter.h"
#include "validator_exception.h"

namespace policy_engine {

bool apply_tag(metadata_factory_t& md_factory, metadata_memory_map_t& map, uint64_t start, uint64_t end, const std::string& tag_name) {
  std::shared_ptr<metadata_t> md = md_factory.lookup_metadata(tag_name);
  if (!md)
    return false;
  map.add_range(start, end, md);
  return true;
}

void md_range(const std::string& policy_dir, const range_map_t& range_map, const std::string& file_name, reporter_t& err) {
  metadata_factory_t md_factory(policy_dir);
  metadata_memory_map_t map;
  
  for (const auto& [ range, tags ] : range_map)
    for (const std::string& tag : tags)
      if (!apply_tag(md_factory, map, range.start, range.end, tag))
        throw std::out_of_range("could not find tag " + tag);
  if (!save_tags(map, file_name))
    throw std::ios::failure("failed write of tag file");
}

} // namespace policy_engine