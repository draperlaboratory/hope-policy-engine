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

#include <stdio.h>
#include <yaml-cpp/yaml.h>
#include "tag_file.h"
#include "elf_utils.h"
#include "basic_elf_io.h"

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

void usage() {
  printf("usage: md_header <elf_file> <soc_file> <tag_file>\n");
}

bool get_soc_ranges(std::string file_name, std::list<range_t> &ranges,
                    reporter_t *err) {
  YAML::Node node = YAML::LoadFile(file_name);
  if (node["SOC"] == NULL) {
    err->error("SOC root node not present\n");
    return false;
  }

  YAML::Node soc = node["SOC"];
  for (YAML::const_iterator it = soc.begin(); it != soc.end(); ++it) {
    range_t range;
    std::string name;

    if(it->second["name"] == NULL) {
      err->error("'name' node not present\n");
      return false;
    }

    // exclude Memory SOC elements, as they're already
    // accounted for by ELF sections
    name = it->second["name"].as<std::string>().c_str();
    if(name.rfind("SOC.Memory", 0) == 0) {
      continue;
    }

    if(it->second["start"] == NULL) {
      err->error("'start' node not present\n");
      return false;
    }
    range.start = it->second["start"].as<address_t>();

    if(it->second["end"] == NULL) {
      err->error("'end' node not present\n");
      return false;
    }
    range.end = it->second["end"].as<address_t>();

    ranges.push_back(range);
  }

  return true;
}

int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *elf_filename;
  FILE *elf_file;
  std::list<Elf_Shdr const *> code_sections;
  std::list<Elf_Shdr const *> data_sections;
  const char *soc_filename;
  std::list<range_t> soc_ranges;
  const char *tag_filename;
  bool is_64_bit = false;

  if (argc < 4) {
    usage();
    return 0;
  }

  elf_filename = argv[1];
  soc_filename = argv[2];
  tag_filename = argv[3];

  elf_file = fopen(elf_filename, "rb");
  if(elf_file == NULL) {
    err.error("Failed to open ELF file\n");
    return 1;
  }
  FILE_reader_t elf_reader(elf_file);
  elf_image_t elf_image(&elf_reader, &err);
  elf_image.load();
  get_elf_sections(&elf_image, code_sections, data_sections);
  if(elf_image.get_ehdr().e_ident[EI_CLASS] == ELFCLASS64) {
    is_64_bit = true;
  }

  get_soc_ranges(soc_filename, soc_ranges, &err);

  if(write_headers(code_sections, data_sections, soc_ranges, is_64_bit, std::string(tag_filename)) == false) {
    err.error("Failed to write headers to tag file\n");
    return 1;
  }

  fclose(elf_file);

  return 0;
}
