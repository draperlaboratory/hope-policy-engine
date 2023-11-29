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

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <utility>
#include "elf_loader.h"
#include "metadata.h"
#include "metadata_factory.h"
#include "metadata_index_map.h"
#include "metadata_memory_map.h"
#include "metadata_register_map.h"
#include "platform_types.h"
#include "policy_meta_set.h"
#include "register_name_map.h"
#include "reporter.h"
#include "tag_file.h"
#include "uleb.h"
#include "yaml_tools.h"

namespace policy_engine {

bool write_headers(
  uleb_writer_t& writer,
  std::list<range_t>& code_ranges,
  std::list<std::pair<range_t, uint8_t>>& data_ranges,
  bool is_64_bit
) {
  if (!writer.write_uleb<uint8_t>(is_64_bit))
    return false;

  if (!writer.write_uleb<uint32_t>(code_ranges.size()))
    return false;
  for (const range_t& range : code_ranges) {
    if (!writer.write_uleb<uint64_t>(range.start))
      return false;
    if (!writer.write_uleb<uint64_t>(range.end))
      return false;
  }

  if (!writer.write_uleb<uint32_t>(data_ranges.size()))
    return false;
  for (const auto& [ range, gran ] : data_ranges) {
    if (!writer.write_uleb<uint64_t>(range.start))
      return false;
    if (!writer.write_uleb<uint64_t>(range.end))
      return false;
    if (!writer.write_uleb<uint64_t>(gran))
      return false;
  }

  return true;
}

bool save_tag_indexes(
  uleb_writer_t& writer,
  const std::vector<const metadata_t*>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& memory_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& register_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& csr_index_map,
  int32_t register_default, int32_t csr_default, int32_t env_default,
  reporter_t& err
) {
  if (!writer.write_uleb<uint32_t>(metadata_values.size()))
    return false;

  for (const metadata_t* v : metadata_values) {
    if (!writer.write_uleb<uint32_t>(v->size()))
      return false;
    for (const meta_t& m : *v)
      if (!writer.write_uleb<meta_t>(m))
        return false;
  }

  if (!writer.write_uleb<uint32_t>(register_index_map.size()))
    return false;
  if (!writer.write_uleb<int32_t>(register_default))
    return false;

  for (const auto& [ name, index ] : register_index_map) {
    std::string register_name = name.substr(name.find_last_of(".") + 1);
    if (register_name_map.find(register_name) == register_name_map.end())
      throw std::out_of_range(register_name);

    if (!writer.write_uleb<uint32_t>(register_name_map.at(register_name)))
      return false;
    if (!writer.write_uleb<uint32_t>(index))
      return false;
  }

  if (!writer.write_uleb<uint32_t>(csr_index_map.size()))
    return false;
  if (!writer.write_uleb<int32_t>(csr_default))
    return false;

  for (const auto& [ name, index ] : csr_index_map) {
    std::string csr_name = name.substr(name.find_last_of(".") + 1);
    if (csr_name_map.find(csr_name) == csr_name_map.end())
      throw std::out_of_range(csr_name);

    if (!writer.write_uleb<uint32_t>(csr_name_map.at(csr_name)))
      return false;
    if (!writer.write_uleb<uint32_t>(index))
      return false;
  }

  if (!writer.write_uleb<int32_t>(env_default))
    return false;

  if (!writer.write_uleb<uint32_t>(memory_index_map.size()))
    return false;
  for (const auto& [ range, index ] : memory_index_map) {
    if (!writer.write_uleb<uint64_t>(range.start))
      return false;
    if (!writer.write_uleb<uint64_t>(range.end))
      return false;
    if (!writer.write_uleb<uint32_t>(index))
      return false;
  }
  return true;
}

void exclude_unused_soc(const YAML::Node& soc, std::list<std::string>& exclude, metadata_factory_t& factory) {
  std::map<std::string, const metadata_t*> soc_map = factory.lookup_metadata_map("SOC");

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
  ranges.sort();
  auto current = std::next(ranges.begin());
  while (current != ranges.end()) {
    err.info("Range: %#lx - %#lx\n", current->start, current->end);

    // this can't actually be bottom-factored, as the iterator has to be incremented before removing data from
    // the list
    auto previous = std::prev(current);
    if (current->end <= previous->end) {
      ranges.erase(current++);
    } else if ((current->start - previous->end) <= 1) {
      previous->end = current->end;
      ranges.erase(current++);
    } else {
      current++;
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
  coalesce_ranges(code_ranges, err);
  coalesce_ranges(data_ranges, err);

  const std::string format_base = "{ 0x%%0%dlx - 0x%%0%dlx }\n";
  char format[format_base.size()];
  std::sprintf(format, format_base.c_str(), elf_image.word_bytes()*2, elf_image.word_bytes()*2);
  err.info("Code ranges:\n");
  for (const range_t& range : code_ranges)
    err.info(format, range.start, range.end);
  err.info("Data ranges:\n");
  for (const range_t& range : data_ranges)
    err.info(format, range.start, range.end);
}

void write_tag_file(
  metadata_factory_t& factory,
  const metadata_memory_map_t& metadata_memory_map,
  const elf_image_t& elf_image,
  const std::string& soc_filename,
  const std::string& tag_filename,
  const std::string& policy_dir,
  const std::list<std::string>&
  soc_exclude,
  reporter_t& err
) {
  if (auto writer = uleb_writer_t(tag_filename)) {
    YAML::Node soc_node = YAML::LoadFile(soc_filename);
    if (!soc_node["SOC"])
      throw std::out_of_range("SOC root node not present");

    std::list<std::string> exclude(soc_exclude);
    exclude_unused_soc(soc_node["SOC"], exclude, factory);

    std::list<range_t> data_ranges = get_soc_ranges(soc_node["SOC"], exclude, err);
    std::list<range_t> code_ranges;
    get_address_ranges(elf_image, code_ranges, data_ranges, err);

    std::list<std::pair<range_t, uint8_t>> data_ranges_granularity;
    for (const range_t& range : data_ranges) {
      data_ranges_granularity.push_back(std::make_pair(range, get_soc_granularity(soc_node["SOC"], range, elf_image.word_bytes())));
    }

    if (!write_headers(writer, code_ranges, data_ranges_granularity, elf_image.word_bytes() == 8))
      throw std::ios::failure("failed to write headers to tag file");

    // Transform (memory/register -> metadata) maps into a metadata list and (memory/register -> index) maps
    std::vector<const metadata_t*> metadata_values;
    metadata_index_map_t<metadata_memory_map_t, range_t> memory_index_map(metadata_memory_map);
    metadata_values.insert(metadata_values.end(), memory_index_map.metadata.begin(), memory_index_map.metadata.end());

    // Add any metadata from initial register/SOC/CSR assignments that's not already in metadata_values
    metadata_index_map_t<metadata_register_map_t, std::string> register_index_map(factory.lookup_metadata_map("ISA.RISCV.Reg"), metadata_values);
    for(const auto& metadata : register_index_map.metadata){
      const auto it = std::find_if(metadata_values.begin(), metadata_values.end(), [&metadata](const metadata_t* v){ return *v == *metadata; });
      if (it == metadata_values.end())
          metadata_values.push_back(metadata);
    }

    metadata_index_map_t<metadata_register_map_t, std::string> soc_index_map(factory.lookup_metadata_map("SOC"), metadata_values);
    for(const auto& metadata : soc_index_map.metadata){
      const auto it = std::find_if(metadata_values.begin(), metadata_values.end(), [&metadata](const metadata_t* v){ return *v == *metadata; });
      if (it == metadata_values.end())
          metadata_values.push_back(metadata);
    }

    metadata_index_map_t<metadata_register_map_t, std::string> csr_index_map(factory.lookup_metadata_map("ISA.RISCV.CSR"), metadata_values);
    for(const auto& metadata : csr_index_map.metadata){
      const auto it = std::find_if(metadata_values.begin(), metadata_values.end(), [&metadata](const metadata_t* v){ return *v == *metadata; });
      if (it == metadata_values.end())
          metadata_values.push_back(metadata);
    }

    // Separate the default entries from those corresponding to actual registers/CSRs
    int register_default = -1;
    if (auto it = register_index_map.find("ISA.RISCV.Reg.Default"); it != register_index_map.end()) {
      register_default = it->second;
      register_index_map.erase(it);
    }

    int csr_default = -1;
    if (auto it = csr_index_map.find("ISA.RISCV.CSR.Default"); it != csr_index_map.end()) {
      csr_default = it->second;
      csr_index_map.erase(it);
    }

    int env_default = register_index_map.at("ISA.RISCV.Reg.Env");
    register_index_map.erase("ISA.RISCV.Reg.Env");

    err.info("Metadata entries:\n");
    for (std::size_t i = 0; i < metadata_values.size(); i++) {
      err.info("%lu: { ", i);
      for (const meta_t& m : *metadata_values[i]) {
        err.info("%lx ", m);
      }
      err.info("}\n");
    }

    if (!save_tag_indexes(writer, metadata_values, memory_index_map, register_index_map, csr_index_map, register_default, csr_default, env_default, err))
      throw std::ios::failure("failed to save indexes to tag file");
  } else {
    throw std::ios::failure("could not open " + tag_filename + " for writing");
  }
}

bool save_metadata(const metadata_memory_map_t& map, uint32_t xlen, const std::string& filename) {
  if (auto writer = uleb_writer_t(filename)) {
    if (!writer.write_uleb<uint32_t>(xlen))
      return false;
    for (const auto& [ range, md ] : map) {
      if (!writer.write_uleb<uint64_t>(range.start))
        return false;
      if (!writer.write_uleb<uint64_t>(range.end))
        return false;
      if (!writer.write_uleb<uint64_t>(md->size()))
        return false;
      for (const meta_t& m : *md) {
        if (!writer.write_uleb<meta_t>(m))
          return false;
      }
    }
  }
  return true;
}

bool load_metadata(metadata_memory_map_t& map, const std::string& file_name, uint32_t& xlen) {
  uleb_reader_t reader(file_name);
  if (!reader)
    return false;

  if (reader.read_uleb<uint32_t>(xlen) <= 0)
    return false;

  while (!reader.eof()) {
    uint64_t start, end;
    uint32_t metadata_count;

    if (reader.read_uleb<uint64_t>(start) <= 0)
      return false;
    if (reader.read_uleb<uint64_t>(end) <= 0)
      return false;
    if (reader.read_uleb<uint32_t>(metadata_count) <= 0)
      return false;
    metadata_t metadata;
    for (uint32_t i = 0; i < metadata_count; i++) {
      meta_t meta;
      if (reader.read_uleb<meta_t>(meta) <= 0)
        return false;
      metadata.insert(meta);
    }
    map.add_range(start, end, metadata);
  }
  return true;
}

bool load_firmware_tag_file(
  std::list<range_t>& code_ranges,
  std::list<range_t>& data_ranges,
  std::vector<metadata_t>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& metadata_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& register_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& csr_index_map,
  const std::string& file_name,
  reporter_t& err,
  int32_t& register_default, int32_t& csr_default, int32_t& env_default
) {
  uint8_t is_64_bit;
  uint32_t code_range_count;
  uint32_t data_range_count;
  uint32_t metadata_value_count;
  uint32_t memory_index_count;
  uint32_t register_index_count;
  uint32_t csr_index_count;

  uleb_reader_t reader(file_name);
  if (!reader)
    return false;

  if (reader.read_uleb<uint8_t>(is_64_bit) <= 0)
    return false;

  if (reader.read_uleb<uint32_t>(code_range_count) <= 0)
    return false;
  for (size_t i = 0; i < code_range_count; i++) {
    range_t range;
    if (reader.read_uleb<uint64_t>(range.start) <= 0)
      return false;
    if (reader.read_uleb<uint64_t>(range.end) <= 0)
      return false;
    code_ranges.push_back(range);
  }

  if (reader.read_uleb<uint32_t>(data_range_count) <= 0)
    return false;
  for (size_t i = 0; i < data_range_count; i++) {
    range_t range;
    if (reader.read_uleb<uint64_t>(range.start) <= 0)
      return false;
    if (reader.read_uleb<uint64_t>(range.end) <= 0)
      return false;
    data_ranges.push_back(range);
    std::size_t tag_granularity;
    if (reader.read_uleb<std::size_t>(tag_granularity) <= 0)
      return false;
  }

  if (reader.read_uleb<uint32_t>(metadata_value_count) <= 0)
    return false;
  for (size_t i = 0; i < metadata_value_count; i++) {
    uint32_t metadata_count;
    if (reader.read_uleb<uint32_t>(metadata_count) <= 0)
      return false;

    metadata_t metadata;
    for (size_t j = 0; j < metadata_count; j++) {
      meta_t meta;
      if (reader.read_uleb<meta_t>(meta) <= 0)
        return false;
      metadata.insert(meta);
    }
    metadata_values.push_back(metadata);
  }

  if (reader.read_uleb<uint32_t>(register_index_count) <= 0)
    return false;
  if (reader.read_uleb<int32_t>(register_default) <= 0)
    return false;
  for (std::size_t i = 0; i < register_index_count; i++) {
    std::string register_name;
    uint32_t register_value;
    uint32_t register_meta;

    if (reader.read_uleb<uint32_t>(register_value) <= 0)
      return false;
    if (reader.read_uleb<uint32_t>(register_meta) <= 0)
      return false;

    const auto it = std::find_if(register_name_map.begin(), register_name_map.end(), [&](const std::pair<std::string, uint32_t>& e){ return e.second == register_value; });
    if (it == register_name_map.end())
      return false;

    std::pair<std::string, uint32_t> p(it->first, register_meta);
    register_index_map.insert(std::make_pair(it->first, register_meta));
  }

  if (reader.read_uleb<uint32_t>(csr_index_count) <= 0)
    return false;
  if (reader.read_uleb<int32_t>(csr_default) <= 0)
    return false;
  for (std::size_t i = 0; i < csr_index_count; i++) {
    uint32_t csr_value;
    uint32_t csr_meta;
    if (reader.read_uleb<uint32_t>(csr_value) <= 0)
      return false;
    if (reader.read_uleb<uint32_t>(csr_meta) <= 0)
      return false;

    const auto it = std::find_if(csr_name_map.begin(), csr_name_map.end(), [&](const std::pair<std::string, uint32_t>& e){ return e.second == csr_value; });
    if (it == csr_name_map.end())
      return false;

    csr_index_map.insert(std::make_pair(it->first, csr_meta));
  }

  if (reader.read_uleb<int32_t>(env_default) <= 0)
    return false;

  if (reader.read_uleb<uint32_t>(memory_index_count) <= 0)
    return false;
  for (size_t i = 0; i < memory_index_count; i++) {
    range_t range;
    uint32_t metadata_index;

    if (reader.read_uleb<uint64_t>(range.start) <= 0)
      return false;
    if (reader.read_uleb<uint64_t>(range.end) <= 0)
      return false;
    if (reader.read_uleb<uint32_t>(metadata_index) <= 0)
      return false;
    metadata_index_map.insert(std::make_pair(range, metadata_index));
  }

  return true;
}

}
