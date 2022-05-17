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
#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unistd.h>
#include "elf_loader.h"
#include "metadata.h"
#include "metadata_factory.h"
#include "metadata_index_map.h"
#include "metadata_register_map.h"
#include "reporter.h"
#include "tag_file.h"

namespace policy_engine {

static const std::string riscv_prefix = "riscv64-unknown-elf-";

void save_tags_to_temp(
  const std::vector<std::shared_ptr<metadata_t>>& metadata_values,
  const metadata_index_map_t<metadata_memory_map_t, range_t>& memory_index_map,
  const elf_image_t& img,
  const std::string& tag_map,
  reporter_t& err
) {
  std::ofstream section_file(tag_map, std::ios::binary);
  int address_width = img.word_bytes()/sizeof(std::ofstream::char_type);

  uint64_t mem_map_size = memory_index_map.size();
  section_file.write(reinterpret_cast<const char*>(&mem_map_size), address_width);
  for (const auto& [ range, index ] : memory_index_map) {
    uint64_t metadata_size = metadata_values[index]->size();
    section_file.write(reinterpret_cast<const char*>(&range.start), address_width);
    section_file.write(reinterpret_cast<const char*>(&range.end), address_width);
    section_file.write(reinterpret_cast<const char*>(&metadata_size), address_width);

    for (const meta_t& m : *metadata_values[index])
      section_file.write(reinterpret_cast<const char*>(&m), address_width);
  }
}

bool embed_tags_in_elf(
  const std::vector<std::shared_ptr<metadata_t>>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& memory_index_map,
  elf_image_t& old_elf,
  const std::string& new_elf_name,
  bool update,
  reporter_t& err
) {
  const std::string section_temp_file = "initial_tag_map";
  save_tags_to_temp(metadata_values, memory_index_map, old_elf, section_temp_file, err);

  char command_string[512];
  const char base_command[] = "%sobjcopy --target elf%d-littleriscv --%s-section .initial_tag_map=%s %s %s %s";
  std::sprintf(command_string, base_command,
    riscv_prefix.c_str(),
    old_elf.word_bytes()*8,
    update ? "update" : "add",
    section_temp_file.c_str(),
    update ? "" : "--set-section-flags .initial_tag_map=readonly,data",
    old_elf.name.c_str(), new_elf_name.c_str()
  );

  return system(command_string) == 0;
}

int md_embed(const std::string& tag_filename, const std::string& policy_dir, elf_image_t& img, const std::string& elf_filename, reporter_t& err) {
  metadata_memory_map_t metadata_memory_map;

  // Retrieve memory metadata from tag file
  if (!load_tags(metadata_memory_map, tag_filename.c_str())) {
    err.error("Failed to load tags\n");
    return 1;
  }

  // Retrieve register metadata from policy
  metadata_factory_t metadata_factory(policy_dir);

  // Transform (memory/register -> metadata) maps into a metadata list and (memory/register -> index) maps
  metadata_index_map_t<metadata_memory_map_t, range_t> memory_index_map(metadata_memory_map);

  // Figure out if the section already exists in the elf. This affects the exact command needed to update the elf.
  const char base_command[] = "%sobjdump --target elf%d-littleriscv -d -j .initial_tag_map %s >/dev/null 2>&1";
  char command_string[256];
  std::sprintf(command_string, base_command, riscv_prefix.c_str(), img.word_bytes()*8, elf_filename.c_str());
  int ret = std::system(command_string);

  if (!embed_tags_in_elf(memory_index_map.metadata, memory_index_map, img, elf_filename, ret == 0, err)) {
    err.error("Failed to save indexes to tag file\n");
    return 1;
  }
  return 0;
}

} // namespace policy_engine