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
#include <stdexcept>
#include <sstream>
#include <string>
#include <unistd.h>
#include "elf_loader.h"
#include "metadata.h"
#include "metadata_index_map.h"
#include "metadata_memory_map.h"
#include "metadata_register_map.h"
#include "range.h"
#include "reporter.h"
#include "tag_file.h"

namespace policy_engine {

static const std::string riscv_prefix = "riscv64-unknown-elf-";

void save_tags_to_temp(
  const std::vector<const metadata_t*>& metadata_values,
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
  const std::vector<const metadata_t*>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& memory_index_map,
  elf_image_t& old_elf,
  const std::string& new_elf_name,
  bool update,
  reporter_t& err
) {
  const std::string section_temp_file = "initial_tag_map";
  save_tags_to_temp(metadata_values, memory_index_map, old_elf, section_temp_file, err);

  std::string command_string;
  command_string += riscv_prefix;
  command_string += std::string("objcopy --target elf") + std::to_string(old_elf.word_bytes()*8) + "-littleriscv";
  command_string += std::string(" --") + (update ? "update" : "add") + "-section .initial_tag_map=";
  command_string += section_temp_file;
  if (update) command_string += " --set-section-flags .initial_tag_map=readonly,data";
  command_string += std::string(" ") + old_elf.name + " " + new_elf_name;
  command_string += " > /dev/null 2>&1";

  return system(command_string.c_str()) == 0;
}

void embed_tags(metadata_memory_map_t& metadata_memory_map, elf_image_t& img, const std::string& elf_filename, reporter_t& err) {
  // Transform (memory/register -> metadata) maps into a metadata list and (memory/register -> index) maps
  metadata_index_map_t<metadata_memory_map_t, range_t> memory_index_map(metadata_memory_map);

  // Figure out if the section already exists in the elf. This affects the exact command needed to update the elf.
  std::string command_string;
  command_string += riscv_prefix;
  command_string += std::string("objdump --target elf") + std::to_string(img.word_bytes()*8) + "-littleriscv";
  command_string += " -d -j .initial_tag_map ";
  command_string += elf_filename;
  command_string += " > /dev/null 2>&1";
  int ret = std::system(command_string.c_str());

  if (!embed_tags_in_elf(memory_index_map.metadata, memory_index_map, img, elf_filename, ret == 0, err))
    throw std::ios::failure("failed to save indices to tag file");
}

} // namespace policy_engine