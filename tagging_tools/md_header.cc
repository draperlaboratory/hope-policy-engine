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
#include <yaml-cpp/yaml.h>
#include "tag_file.h"
#include "elf_utils.h"
#include "basic_elf_io.h"
#include "metadata_factory.h"

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

void usage() {
  printf("usage: md_header <elf_file> <soc_file> <tag_file>\n");
}

bool exclude_unused_soc(YAML::Node soc, std::list<std::string> &exclude,
                        metadata_factory_t &factory, reporter_t *err) {
  auto soc_map = factory.lookup_metadata_map("SOC");

  for (YAML::const_iterator it = soc.begin(); it != soc.end(); ++it) {
    std::string name;

    if(it->second["name"] == NULL) {
      err->error("'name' node not present\n");
      return false;
    }

    name = it->second["name"].as<std::string>();

    if(soc_map->find(name) == soc_map->end()) {
      exclude.push_back(name);
    }

  }

  return true;
}

bool get_soc_ranges(YAML::Node soc, std::list<range_t> &ranges,
                    std::list<std::string> &exclude, reporter_t *err) {
  for (YAML::const_iterator it = soc.begin(); it != soc.end(); ++it) {
    range_t range;
    std::string name;

    if(it->second["name"] == NULL) {
      err->error("'name' node not present\n");
      return false;
    }

    name = it->second["name"].as<std::string>();

    if(std::find(exclude.begin(), exclude.end(), name) != exclude.end()) {
      err->info("Excluding %s from SOC ranges\n", name.c_str());
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

void elf_sections_to_ranges(std::list<Elf_Shdr const *> &sections,
                            std::list<range_t> &ranges) {
  for(const auto &it : sections) {
    range_t range;
    range.start = it->sh_addr;

    // Round up to the next word boundary, then subtract 1 to make inclusive
    range.end = it->sh_addr + it->sh_size;
    range.end += (4 - (range.end % 4)) % 4;
    range.end -= 1;

    ranges.push_back(range);
  }
}

bool compare_range(range_t &first, range_t &second) {
  return (first.start < second.start);
}

void coalesce_ranges(std::list<range_t> &ranges) {
  std::list<range_t>::iterator it = ++ranges.begin();

  while(it != ranges.end()) {
#ifdef RV64_VALIDATOR
    printf("Range: 0x%lx = 0x%lx\n", (*it).start, (*it).end);
#else
    printf("Range: 0x%x = 0x%x\n", (*it).start, (*it).end);
#endif
    auto previous = std::prev(it, 1);

    if((*it).end <= (*previous).end) {
      ranges.erase(it++);
      continue;
    }

    if(((*it).start - (*previous).end) <= 1) {
      (*previous).end = (*it).end;
      ranges.erase(it++);
      continue;
    }

    it++;
  }
}

void get_address_ranges(elf_image_t &elf_image,
                        std::list<range_t> &code_ranges,
                        std::list<range_t> &data_ranges) {
  std::list<Elf_Shdr const *> code_sections;
  std::list<Elf_Shdr const *> data_sections;

  get_elf_sections(&elf_image, code_sections, data_sections);

  elf_sections_to_ranges(code_sections, code_ranges);
  elf_sections_to_ranges(data_sections, data_ranges);

  code_ranges.sort(compare_range);
  data_ranges.sort(compare_range);

  coalesce_ranges(code_ranges);
  coalesce_ranges(data_ranges);

  printf("Code ranges:\n");
  for(const auto &it : code_ranges) {
#ifdef RV64_VALIDATOR
    printf("{ 0x%08lx - 0x%08lx }\n", it.start, it.end);
#else
    printf("{ 0x%08x - 0x%08x }\n", it.start, it.end);
#endif
  }

  printf("Data ranges:\n");
  for(const auto &it : data_ranges) {
#ifdef RV64_VALIDATOR
    printf("{ 0x%08lx - 0x%08lx }\n", it.start, it.end);
#else
    printf("{ 0x%08x - 0x%08x }\n", it.start, it.end);
#endif
  }
}

int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *elf_filename;
  FILE *elf_file;
  const char *soc_filename;
  std::list<range_t> soc_ranges;
  std::list<range_t> code_ranges;
  std::list<range_t> data_ranges;
  const char *tag_filename;
  bool is_64_bit = false;
  std::list<std::string> soc_exclude;
  const char *policy_dir;
  YAML::Node soc_node;

  if (argc < 5) {
    usage();
    return 1;
  }

  elf_filename = argv[1];
  soc_filename = argv[2];
  tag_filename = argv[3];
  policy_dir = argv[4];

  metadata_factory_t factory(policy_dir);

  for(size_t i = 5; i < argc; i++) {
    soc_exclude.push_back(std::string(argv[i]));
  }

  elf_file = fopen(elf_filename, "rb");
  if(elf_file == NULL) {
    err.error("Failed to open ELF file\n");
    return 1;
  }
  FILE_reader_t elf_reader(elf_file);
  elf_image_t elf_image(&elf_reader, &err);
  if(elf_image.load() == false) {
    err.error("Failed to load ELF image\n");
    return 1;
  }

  if(elf_image.get_ehdr().e_ident[EI_CLASS] == ELFCLASS64) {
    is_64_bit = true;
  }

  soc_node = YAML::LoadFile(soc_filename);
  if (soc_node["SOC"] == NULL) {
    err.error("SOC root node not present\n");
    return false;
  }

  if(exclude_unused_soc(soc_node["SOC"], soc_exclude, factory, &err) == false) {
    err.error("Failed to get SOC ranges\n");
    return 1;
  }

  if(get_soc_ranges(soc_node["SOC"], soc_ranges, soc_exclude, &err) == false) {
    err.error("Failed to get SOC ranges\n");
    return 1;
  }

  data_ranges.insert(data_ranges.end(), soc_ranges.begin(), soc_ranges.end());
  get_address_ranges(elf_image, code_ranges, data_ranges);

  if(write_headers(code_ranges, data_ranges, is_64_bit, std::string(tag_filename)) == false) {
    err.error("Failed to write headers to tag file\n");
    return 1;
  }

  fclose(elf_file);

  return 0;
}
