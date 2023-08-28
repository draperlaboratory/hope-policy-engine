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

#ifndef TAG_FILE_H
#define TAG_FILE_H

#include <iostream>
#include <list>
#include <string>
#include "elf_loader.h"
#include "metadata_factory.h"
#include "metadata_index_map.h"
#include "metadata_memory_map.h"
#include "metadata_register_map.h"
#include "range.h"
#include "reporter.h"

namespace policy_engine {

void write_tag_file(
  metadata_factory_t& factory,
  const metadata_memory_map_t& metadata_memory_map,
  const elf_image_t& elf_image,
  const std::string& soc_filename,
  const std::string& tag_filename,
  const std::string& policy_dir,
  const std::list<std::string>& soc_exclude,
  reporter_t& err
);

bool save_metadata(const metadata_memory_map_t& map, uint32_t xlen, const std::string& filename);
bool load_metadata(metadata_memory_map_t& map, const std::string& file_name, uint32_t& xlen);

template<class OStream>
void dump_tags(const metadata_memory_map_t& map, metadata_factory_t& factory, OStream&& out) {
  out << std::hex;
  for (const auto& [ range, metadata ] : map)
    out << '[' << range.start << ',' << range.end << "]: " << factory.render(metadata, false) << std::endl;
  out << std::dec;
}

bool load_firmware_tag_file(
  std::list<range_t>& code_ranges,
  std::list<range_t>& data_ranges,
  std::vector<metadata_t>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& metadata_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& register_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& csr_index_map,
  const std::string& file_name,
  reporter_t& err,
  int32_t& register_default, int32_t& csr_default, int32_t& env_default
);

} // namespace policy_engine

#endif
