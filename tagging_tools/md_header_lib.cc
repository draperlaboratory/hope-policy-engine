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
#include <map>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <yaml-cpp/yaml.h>
#include "elf_loader.h"
#include "metadata.h"
#include "metadata_factory.h"
#include "reporter.h"
#include "tag_file.h"
#include "tagging_utils.h"

namespace policy_engine {

void exclude_unused_soc(const YAML::Node& soc, std::list<std::string>& exclude, metadata_factory_t& factory) {
  std::map<std::string, std::shared_ptr<metadata_t>> soc_map = factory.lookup_metadata_map("SOC");

  for (const auto& node : soc) {
    if (!node.second["name"])
      throw std::out_of_range("md_header: 'name' node not present");
    std::string name = node.second["name"].as<std::string>();
    if (soc_map.find(name) == soc_map.end()) {
      exclude.push_back(name);
    }
  }
}

std::list<range_t> get_soc_ranges(const YAML::Node& soc, const std::list<std::string>& exclude, reporter_t& err) {
  std::list<range_t> ranges;
  for (const auto& node : soc) {
    if (!node.second["name"])
      throw std::out_of_range("md_header: 'name' node not present");
    std::string name = node.second["name"].as<std::string>();

    if (std::find(exclude.begin(), exclude.end(), name) != exclude.end()) {
      err.info("Excluding %s from SOC ranges\n", name);
      continue;
    }

    if (!node.second["start"])
      throw std::out_of_range("md_header: 'start' node not present");
    if (!node.second["end"])
      throw std::out_of_range("md_header: 'end' node not present");
    ranges.push_back(range_t{node.second["start"].as<uint64_t>(), node.second["end"].as<uint64_t>()});
  }
  return ranges;
}

std::size_t get_soc_granularity(const YAML::Node& soc, range_t range, std::size_t default_granularity) {
  for (const auto& node: soc) {
    uint64_t start = node.second["start"].as<uint64_t>();
    uint64_t end = node.second["end"].as<uint64_t>();
    if (start == range.start && end == range.end) {
      if (node.second["tag_granularity"])
        return node.second["tag_granularity"].as<size_t>();
      return default_granularity;
    }
  }
  return default_granularity;
}

void elf_sections_to_ranges(const std::list<const elf_section_t*>& sections, std::list<range_t>& ranges) {
  for (const auto section : sections)
    ranges.push_back(range_t{section->address, round_up(section->address + section->size, 4) - 1});
}

void coalesce_ranges(std::list<range_t>& ranges, reporter_t& err) {
  std::list<range_t>::iterator it = ++ranges.begin();

  while(it != ranges.end()) {
    err.info("Range: 0x%lx = 0x%lx\n", it->start, it->end);

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

void get_address_ranges(elf_image_t& elf_image, std::list<range_t>& code_ranges, std::list<range_t>& data_ranges, reporter_t& err) {
  std::list<const elf_section_t*> code_sections;
  std::list<const elf_section_t*> data_sections;

  for (const elf_section_t& section : elf_image.sections) {
    if (section.flags & SHF_ALLOC) {
      if ((section.flags & (SHF_WRITE | SHF_EXECINSTR)) == SHF_EXECINSTR)
        code_sections.push_back(&section);
      else
        data_sections.push_back(&section);
    }
  }

  elf_sections_to_ranges(code_sections, code_ranges);
  elf_sections_to_ranges(data_sections, data_ranges);

  code_ranges.sort();
  data_ranges.sort();

  coalesce_ranges(code_ranges, err);
  coalesce_ranges(data_ranges, err);

  err.info("Code ranges:\n");
  if (elf_image.word_bytes() == 8) {
    for (const range_t& range : code_ranges)
      err.info("{ 0x%016lx - 0x%016lx }\n", range.start, range.end);
  } else {
    for (const range_t& range : code_ranges)
      err.info("{ 0x%08lx - 0x%08lx }\n", range.start, range.end);
  }

  err.info("Data ranges:\n");
  if (elf_image.word_bytes() == 8) {
    for(const range_t& range : data_ranges)
      err.info("{ 0x%016lx - 0x%016lx }\n", range.start, range.end);
  } else {
    for(const range_t& range : data_ranges)
      err.info("{ 0x%08lx - 0x%08lx }\n", range.start, range.end);
  }
}

void md_header(const std::string& elf_filename, const std::string& soc_filename, const std::string& tag_filename, const std::string& policy_dir, std::list<std::string>& soc_exclude, reporter_t& err) {
  metadata_factory_t factory(policy_dir);
  elf_image_t elf_image(elf_filename);

  YAML::Node soc_node = YAML::LoadFile(soc_filename);
  if (!soc_node["SOC"])
    throw std::out_of_range("md_header: SOC root node not present");

  exclude_unused_soc(soc_node["SOC"], soc_exclude, factory);

  std::list<range_t> soc_ranges = get_soc_ranges(soc_node["SOC"], soc_exclude, err);

  std::list<range_t> code_ranges, data_ranges;
  data_ranges.insert(data_ranges.end(), soc_ranges.begin(), soc_ranges.end());
  get_address_ranges(elf_image, code_ranges, data_ranges, err);

  std::list<std::pair<range_t, uint8_t>> data_ranges_granularity;
  for (const range_t& range : data_ranges) {
    data_ranges_granularity.push_back(std::make_pair(range, get_soc_granularity(soc_node["SOC"], range, elf_image.word_bytes())));
  }

  if (!write_headers(code_ranges, data_ranges_granularity, elf_image.word_bytes() == 8, std::string(tag_filename)))
    throw std::ios::failure("md_header: failed to write headers to tag file");
}

} // namespace policy_engine