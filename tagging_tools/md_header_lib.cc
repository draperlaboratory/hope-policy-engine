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
#include "yaml_tools.h"

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
    } else {
      if (!node.second["start"])
        throw std::out_of_range("md_header: 'start' node not present");
      if (!node.second["end"])
        throw std::out_of_range("md_header: 'end' node not present");
      ranges.push_back(node.second.as<range_t>());
    }
  }
  return ranges;
}

std::size_t get_soc_granularity(const YAML::Node& soc, const range_t& range, std::size_t default_granularity) {
  for (const auto& node: soc) {
    range_t current{node.second["start"].as<uint64_t>(), node.second["end"].as<uint64_t>()};
    if (current == range) {
      if (node.second["tag_granularity"])
        return node.second["tag_granularity"].as<std::size_t>();
      return default_granularity;
    }
  }
  return default_granularity;
}

void coalesce_ranges(std::list<range_t>& ranges, reporter_t& err) {
  auto it = std::next(ranges.begin());
  while (it != ranges.end()) {
    err.info("Range: 0x%lx = 0x%lx\n", it->start, it->end);

    // this can't actually be bottom-factored, as the iterator has to be incremented before removing data from
    // the list
    auto previous = std::prev(it);
    if (it->end <= previous->end) {
      ranges.erase(it++);
    } else if ((it->start - previous->end) <= 1) {
      previous->end = it->end;
      ranges.erase(it++);
    } else {
      it++;
    }
  }
}

void get_address_ranges(const elf_image_t& elf_image, std::list<range_t>& code_ranges, std::list<range_t>& data_ranges, reporter_t& err) {

  for (const elf_section_t& section : elf_image.sections) {
    if (section.flags & SHF_ALLOC) {
      if ((section.flags & (SHF_WRITE | SHF_EXECINSTR)) == SHF_EXECINSTR)
        code_ranges.push_back(section.address_range());
      else
        data_ranges.push_back(section.address_range());
    }
  }
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

void md_header(metadata_factory_t& factory, const elf_image_t& elf_image, const std::string& soc_filename, const std::string& tag_filename, const std::string& policy_dir, std::list<std::string>& soc_exclude, reporter_t& err) {

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