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
#include <sstream>
#include <string>
#include "metadata_index_map.h"
#include "metadata_factory.h"
#include "metadata_register_map.h"
#include "reporter.h"
#include "tag_file.h"

namespace policy_engine {

int md_index(const std::string& tag_filename, const std::string& policy_dir, reporter_t& err) {
  metadata_memory_map_t metadata_memory_map;
  metadata_register_map_t *register_map;
  metadata_register_map_t *csr_map;
  std::vector<const metadata_t *> metadata_values;
  ssize_t register_default = -1;
  ssize_t csr_default = -1;
  ssize_t env_default = -1;

  // Retrieve memory metadata from tag file
  if(load_tags(&metadata_memory_map, tag_filename) == false) {
    err.error("Failed to load tags\n");
    return 1;
  }

  // Retrieve register metadata from policy
  metadata_factory_t metadata_factory(policy_dir);

  register_map = metadata_factory.lookup_metadata_map("ISA.RISCV.Reg");
  csr_map = metadata_factory.lookup_metadata_map("ISA.RISCV.CSR");

  // Transform (memory/register -> metadata) maps into a metadata list and (memory/register -> index) maps
  auto memory_index_map =
    metadata_index_map_t<metadata_memory_map_t, range_t>(&metadata_memory_map, &metadata_values);

  auto register_index_map =
    metadata_index_map_t<metadata_register_map_t, std::string>(register_map, &metadata_values);

  auto csr_index_map =
    metadata_index_map_t<metadata_register_map_t, std::string>(csr_map, &metadata_values);

  // Separate the default entries from those corresponding to actual registers/CSRs
  try {
    register_default = register_index_map.at("ISA.RISCV.Reg.Default");
    register_index_map.erase("ISA.RISCV.Reg.Default");
  } catch(const std::out_of_range& oor) { }

  try {
    csr_default = csr_index_map.at("ISA.RISCV.CSR.Default");
    csr_index_map.erase("ISA.RISCV.CSR.Default");
  } catch(const std::out_of_range& oor) { }

  env_default = register_index_map.at("ISA.RISCV.Reg.Env");
  register_index_map.erase("ISA.RISCV.Reg.Env");

  err.info("Metadata entries:\n");
  for(size_t i = 0; i < metadata_values.size(); i++) {
    err.info("%lu: { ", i);
    for(const auto &m : *metadata_values[i]) {
      err.info("%lx ", m);
    }
    err.info("}\n");
  }

  if (save_tag_indexes(metadata_values, memory_index_map, register_index_map, csr_index_map,
                      register_default, csr_default, env_default, tag_filename, err) == false) {
    err.error("Failed to save indexes to tag file\n");
    return 1;
  }

  return 0;
}

} // namespace policy_engine