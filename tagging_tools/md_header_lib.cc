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
#include <functional>
#include <gelf.h>
#include <sstream>
#include <string>
#include <yaml-cpp/yaml.h>
#include "basic_elf_io.h"
#include "elf_utils.h"
#include "metadata_factory.h"
#include "tag_file.h"

namespace policy_engine {

bool exclude_unused_soc(YAML::Node soc, std::list<std::string>& exclude, metadata_factory_t& factory, reporter_t* err) {
  auto soc_map = factory.lookup_metadata_map("SOC");

  for (YAML::const_iterator it = soc.begin(); it != soc.end(); ++it) {
    std::string name;

    if (it->second["name"] == NULL) {
      err->error("'name' node not present\n");
      return false;
    }

    name = it->second["name"].as<std::string>();

    if (soc_map->find(name) == soc_map->end()) {
      exclude.push_back(name);
    }
  }

  return true;
}

bool get_soc_ranges(YAML::Node soc, std::list<range_t>& ranges, const std::list<std::string>& exclude, reporter_t* err) {
  for (YAML::const_iterator it = soc.begin(); it != soc.end(); ++it) {
    range_t range;
    std::string name;

    if (it->second["name"] == NULL) {
      err->error("'name' node not present\n");
      return false;
    }

    name = it->second["name"].as<std::string>();

    if (std::find(exclude.begin(), exclude.end(), name) != exclude.end()) {
      err->info("Excluding %s from SOC ranges\n", name.c_str());
      continue;
    }

    if (it->second["start"] == NULL) {
      err->error("'start' node not present\n");
      return false;
    }
    range.start = it->second["start"].as<uint64_t>();

    if (it->second["end"] == NULL) {
      err->error("'end' node not present\n");
      return false;
    }
    range.end = it->second["end"].as<uint64_t>();

    ranges.push_back(range);
  }

  return true;
}

size_t get_soc_granularity(YAML::Node soc, range_t range, bool is_64_bit) {
  size_t default_granularity = is_64_bit ? 8 : 4;
  for (YAML::const_iterator it = soc.begin(); it != soc.end(); ++it) {
    uint64_t start = it->second["start"].as<uint64_t>();
    uint64_t end = it->second["end"].as<uint64_t>();
    if (start ==  range.start && end == range.end) {
      if (it->second["tag_granularity"] != NULL) {
        return it->second["tag_granularity"].as<size_t>();
      }
      return default_granularity;
    }
  }

  return default_granularity;
}

void elf_sections_to_ranges(const std::list<const elf_section_t*>& sections, std::list<range_t>& ranges) {
  for(const auto section : sections) {
    range_t range;
    range.start = section->address;

    // Round up to the next word boundary, then subtract 1 to make inclusive
    range.end = section->address + section->size;
    range.end += (4 - (range.end % 4)) % 4;
    range.end -= 1;

    ranges.push_back(range);
  }
}

bool compare_range(range_t& first, range_t& second) {
  return (first.start < second.start);
}

void coalesce_ranges(std::list<range_t>& ranges) {
  std::list<range_t>::iterator it = ++ranges.begin();

  while(it != ranges.end()) {
    std::printf("Range: 0x%lx = 0x%lx\n", it->start, it->end);

    auto previous = std::prev(it, 1);

    if (it->end <= previous->end) {
      ranges.erase(it++);
      continue;
    }

    if ((it->start - previous->end) <= 1) {
      previous->end = it->end;
      ranges.erase(it++);
      continue;
    }

    it++;
  }
}

void get_address_ranges(elf_image_t& elf_image, std::list<range_t>& code_ranges, std::list<range_t>& data_ranges) {
  std::list<const elf_section_t*> code_sections;
  std::list<const elf_section_t*> data_sections;

  for (const auto& section : elf_image.sections) {
    if (section.flags & SHF_ALLOC) {
      if ((section.flags & (SHF_WRITE | SHF_EXECINSTR)) == SHF_EXECINSTR)
        code_sections.push_back(&section);
      else
        data_sections.push_back(&section);
    }
  }

  elf_sections_to_ranges(code_sections, code_ranges);
  elf_sections_to_ranges(data_sections, data_ranges);

  code_ranges.sort(compare_range);
  data_ranges.sort(compare_range);

  coalesce_ranges(code_ranges);
  coalesce_ranges(data_ranges);

  std::printf("Code ranges:\n");
  for(const auto& it : code_ranges) {
    std::printf("{ 0x%016lx - 0x%016lx }\n", it.start, it.end);
  }

  std::printf("Data ranges:\n");
  for(const auto& it : data_ranges) {
    std::printf("{ 0x%016lx - 0x%016lx }\n", it.start, it.end);
  }
}

int md_header(const std::string& elf_filename, const std::string& soc_filename, const std::string& tag_filename, const std::string& policy_dir, std::list<std::string>& soc_exclude, stdio_reporter_t& err) {
  std::list<range_t> soc_ranges;
  std::list<range_t> code_ranges;
  std::list<range_t> data_ranges;
  std::list<std::pair<range_t, uint8_t>> data_ranges_granularity;
  bool is_64_bit = false;
  YAML::Node soc_node;

  metadata_factory_t factory(policy_dir);
  elf_image_t elf_image(elf_filename, err);
  if (!elf_image.is_valid()) {
    err.error("Failed to load ELF image\n");
    return 1;
  }

  if (elf_image.get_ehdr().e_ident[EI_CLASS] == ELFCLASS64) {
    is_64_bit = true;
  }

  soc_node = YAML::LoadFile(soc_filename);
  if (soc_node["SOC"] == NULL) {
    err.error("SOC root node not present\n");
    return false;
  }

  if (exclude_unused_soc(soc_node["SOC"], soc_exclude, factory,& err) == false) {
    err.error("Failed to get SOC ranges\n");
    return 1;
  }

  if (get_soc_ranges(soc_node["SOC"], soc_ranges, soc_exclude,& err) == false) {
    err.error("Failed to get SOC ranges\n");
    return 1;
  }

  data_ranges.insert(data_ranges.end(), soc_ranges.begin(), soc_ranges.end());
  get_address_ranges(elf_image, code_ranges, data_ranges);

  for(const auto& it : data_ranges) {
    data_ranges_granularity.push_back(std::make_pair(it, get_soc_granularity(soc_node["SOC"], it, is_64_bit)));
  }

  if (!write_headers(code_ranges, data_ranges_granularity, is_64_bit, std::string(tag_filename))) {
    err.error("Failed to write headers to tag file\n");
    return 1;
  }

  return 0;
}

} // namespace policy_engine