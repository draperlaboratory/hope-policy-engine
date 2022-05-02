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
#include "metadata_factory.h"
#include "metadata_index_map.h"
#include "metadata_register_map.h"
#include "reporter.h"
#include "tag_file.h"

namespace policy_engine {

static const std::string riscv_prefix = "riscv64-unknown-elf-";

void save_tags_to_temp(
  std::vector<const metadata_t*>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& memory_index_map,
  elf_image_t& img, std::string new_elf_name, char tempfile[],
  bool is_64_bit, reporter_t& err
) {
  int address_width = img.is_64bit() ? 8 : 4;
  int fd = mkstemp(tempfile);
  std::FILE* section_file = fdopen(fd, "wb");

  size_t mem_map_size = memory_index_map.size();
  fwrite(&mem_map_size, address_width, 1, section_file);
  int i = 0;
  for (auto& e : memory_index_map) {
    range_t range = e.first;
    uint64_t metadata_size = metadata_values[e.second]->size();

    fwrite(&range.start, address_width, 1, section_file);
    fwrite(&range.end, address_width, 1, section_file);
    fwrite(&metadata_size, address_width, 1, section_file);

    for (const meta_t& m : *metadata_values[e.second]) {
      fwrite(&m, address_width, 1, section_file);
    }
  }

  fclose(section_file);
  close(fd);
}

bool embed_tags_in_elf(
  std::vector<const metadata_t*>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& memory_index_map,
  elf_image_t& old_elf, std::string new_elf_name, bool update, bool is_64_bit,
  reporter_t& err
) {
  char section_temp_file[] = "/tmp/sectionXXXXXX";
  save_tags_to_temp(metadata_values, memory_index_map, old_elf, new_elf_name, section_temp_file, is_64_bit, err);

  char command_string[512];
  const char* base_command = update ?
    "%sobjcopy --target %s --update-section .initial_tag_map=%s %s %s >/dev/null 2>&1" : 
    "%sobjcopy --target %s --add-section .initial_tag_map=%s --set-section-flags .initial_tag_map=readonly,data %s %s >/dev/null 2>&1";
  std::string bfd_target = is_64_bit ? "elf64-littleriscv" : "elf32-littleriscv";
  std::sprintf(command_string, base_command, riscv_prefix.c_str(), bfd_target.c_str(), section_temp_file, old_elf.name.c_str(), new_elf_name.c_str());
  int ret = system(command_string);

  if (remove(section_temp_file))
    err.warning("Failed to delete temporary file %s.\n", section_temp_file);

  return (ret == 0);
}

int md_embed(const std::string& tag_filename, const std::string& policy_dir, elf_image_t& img, const std::string& elf_filename, bool is_64_bit, reporter_t& err) {
  metadata_memory_map_t metadata_memory_map;
  std::vector<const metadata_t*> metadata_values;

  // Retrieve memory metadata from tag file
  if (!load_tags(&metadata_memory_map, tag_filename.c_str())) {
    err.error("Failed to load tags\n");
    return 1;
  }

  // Retrieve register metadata from policy
  metadata_factory_t metadata_factory(policy_dir);

  // Transform (memory/register -> metadata) maps into a metadata list and (memory/register -> index) maps
  auto memory_index_map = metadata_index_map_t<metadata_memory_map_t, range_t>(&metadata_memory_map, &metadata_values);

  // Figure out if the section already exists in the elf. This affects the exact command needed to update the elf.
  const char base_command[] = "%sobjdump --target %s -d -j .initial_tag_map %s >/dev/null 2>&1";
  std::string bfd_target = is_64_bit ? "elf64-littleriscv" : "elf32-littleriscv";
  char command_string[256];
  std::sprintf(command_string, base_command, riscv_prefix.c_str(), bfd_target.c_str(), elf_filename.c_str());
  int ret = system(command_string);

  if (!embed_tags_in_elf(metadata_values, memory_index_map, img, elf_filename, ret == 0, is_64_bit, err)) {
    err.error("Failed to save indexes to tag file\n");
    return 1;
  }
  return 0;
}

} // namespace policy_engine