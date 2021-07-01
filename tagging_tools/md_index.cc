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

#include <stdio.h>
#include "tag_file.h"
#include "elf_utils.h"
#include "basic_elf_io.h"
#include "metadata_index_map.h"
#include "metadata_factory.h"
#include "metadata_register_map.h"

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

void usage() {
  printf("usage: md_index <tag_file> <policy_dir>\n");
}


int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *tag_filename;
  const char *policy_dir;
  metadata_memory_map_t metadata_memory_map;
  metadata_register_map_t *register_map;
  metadata_register_map_t *csr_map;
  std::vector<const metadata_t *> metadata_values;
  ssize_t register_default = -1;
  ssize_t csr_default = -1;
  ssize_t env_default = -1;

  if(argc < 3) {
    usage();
    return 1;
  }

  // Retrieve memory metadata from tag file
  tag_filename = argv[1];
  if(load_tags(&metadata_memory_map, tag_filename) == false) {
    err.error("Failed to load tags\n");
    return 1;
  }

  // Retrieve register metadata from policy
  policy_dir = argv[2];
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

  printf("Metadata entries:\n");
  for(size_t i = 0; i < metadata_values.size(); i++) {
    printf("%lu: { ", i);
    for(const auto &m : *metadata_values[i]) {
      printf("%lx ", m);
    }
    printf("}\n");
  }

  if(save_tag_indexes(metadata_values, memory_index_map, register_index_map, csr_index_map,
                      register_default, csr_default, env_default, tag_filename) == false) {
    err.error("Failed to save indexes to tag file\n");
    return 1;
  }

  return 0;
}
