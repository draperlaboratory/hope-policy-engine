/* Copyright © 2017-2019 The Charles Stark Draper Laboratory, Inc. and/or Dover Microsystems, Inc. */
/* All rights reserved. */

/* Use and disclosure subject to the following license. */

/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the */
/* "Software"), to deal in the Software without restriction, including */
/* without limitation the rights to use, copy, modify, merge, publish, */
/* distribute, sublicense, and/or sell copies of the Software, and to */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions: */

/* The above copyright notice and this permission notice shall be */
/* included in all copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE */
/* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION */
/* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION */
/* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include "metadata_index_map.h"
#include "metadata_factory.h"
#include "metadata_register_map.h"
#include "policy_meta_set.h"
#include "reporter.h"
#include "tag_file.h"

namespace policy_engine {

void md_index(metadata_factory_t& metadata_factory, metadata_memory_map_t& metadata_memory_map, const std::string& tag_filename, reporter_t& err) {
  std::vector<std::shared_ptr<metadata_t>> metadata_values;

  metadata_register_map_t register_map = metadata_factory.lookup_metadata_map("ISA.RISCV.Reg");
  metadata_register_map_t csr_map = metadata_factory.lookup_metadata_map("ISA.RISCV.CSR");

  // Transform (memory/register -> metadata) maps into a metadata list and (memory/register -> index) maps
  metadata_index_map_t<metadata_memory_map_t, range_t> memory_index_map(metadata_memory_map);
  metadata_values.insert(metadata_values.end(), memory_index_map.metadata.begin(), memory_index_map.metadata.end());
  metadata_index_map_t<metadata_register_map_t, std::string> register_index_map(register_map);
  metadata_values.insert(metadata_values.end(), register_index_map.metadata.begin(), register_index_map.metadata.end());
  metadata_index_map_t<metadata_register_map_t, std::string> csr_index_map(csr_map);
  metadata_values.insert(metadata_values.end(), csr_index_map.metadata.begin(), csr_index_map.metadata.end());

  // Separate the default entries from those corresponding to actual registers/CSRs
  int register_default = -1;
  if (auto it = register_index_map.find("ISA.RISCV.Reg.Default"); it != register_index_map.end()) {
    register_default = it->second;
    register_index_map.erase(it);
  }

  int csr_default = -1;
  if (auto it = csr_index_map.find("ISA.RISCV.CSR.Default"); it != csr_index_map.end()) {
    csr_default = it->second;
    csr_index_map.erase(it);
  }

  int env_default = register_index_map.at("ISA.RISCV.Reg.Env");
  register_index_map.erase("ISA.RISCV.Reg.Env");

  err.info("Metadata entries:\n");
  for(size_t i = 0; i < metadata_values.size(); i++) {
    err.info("%lu: { ", i);
    for(const meta_t& m : *metadata_values[i]) {
      err.info("%lx ", m);
    }
    err.info("}\n");
  }

  if (!save_tag_indexes(metadata_values, memory_index_map, register_index_map, csr_index_map, register_default, csr_default, env_default, tag_filename, err))
    throw std::ios::failure("failed to save indexes to tag file");
}

} // namespace policy_engine