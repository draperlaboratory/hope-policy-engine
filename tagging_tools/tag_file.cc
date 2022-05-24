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
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include "metadata.h"
#include "policy_meta_set.h"
#include "register_name_map.h"
#include "reporter.h"
#include "tag_file.h"
#include "uleb.h"

using namespace policy_engine;

class stream_reader_t {
private:
  std::ifstream is;
  std::streamsize size;

public:
  stream_reader_t(const std::string& fname, std::ios::openmode mode=std::ios::binary) : is(std::ifstream(fname, mode)) {
    is.ignore(std::numeric_limits<std::streamsize>::max());
    size = is.gcount();
    is.clear();
    is.seekg(0, std::ios::beg);
  }

  template<class T> std::streamsize read(T* data, std::streamsize n) {
    try {
      std::streamsize b = n*sizeof(T)/sizeof(std::ofstream::char_type);
      std::ofstream::char_type bytes[b];
      std::streamsize r = is.read(bytes, b).gcount();
      if (r == b)
        std::memcpy(data, bytes, b);
      return r*sizeof(std::ofstream::char_type)/sizeof(T);
    } catch (const std::ios::failure& e) {
      return 0;
    }
  }

  bool read_byte(uint8_t& b) { return read(&b, 1) == 1; }

  std::streamsize length() { return size; }
  bool eof() { return is.tellg() >= size; }

  explicit operator bool() const { return static_cast<bool>(is); }
};

class stream_writer_t {
private:
  std::ofstream os;

public:
  stream_writer_t(const std::string& fname, std::ios::openmode mode=std::ios::binary) : os(std::ofstream(fname, mode)) {}

  template<class T> bool write(const T* data, std::size_t n) {
    try {
      os.write(reinterpret_cast<const std::ofstream::char_type*>(data), n*sizeof(T)/sizeof(std::ofstream::char_type));
      return !os.fail();
    } catch (const std::ios::failure& e) {
      return false;
    }
  }

  bool write_byte(uint8_t b) { return write(&b, 1); }

  explicit operator bool() const { return static_cast<bool>(os); }
};

bool policy_engine::save_tag_indexes(
  std::vector<std::shared_ptr<metadata_t>>& metadata_values,
  metadata_index_map_t<metadata_memory_map_t, range_t>& memory_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& register_index_map,
  metadata_index_map_t<metadata_register_map_t, std::string>& csr_index_map,
  int32_t register_default, int32_t csr_default, int32_t env_default,
  const std::string& file_name,
  reporter_t& err
) {
  stream_writer_t writer(file_name, std::ios::binary | std::ios::app);
  if (!writer)
    return false;

  if (!write_uleb<stream_writer_t, uint32_t>(writer, metadata_values.size()))
    return false;

  for (const std::shared_ptr<metadata_t>& v : metadata_values) {
    if (!write_uleb<stream_writer_t, uint32_t>(writer, v->size()))
      return false;
    for (const meta_t& m : *v)
      if (!write_uleb<stream_writer_t, meta_t>(writer, m))
        return false;
  }

  if (!write_uleb<stream_writer_t, uint32_t>(writer, register_index_map.size()))
    return false;
  if (!write_uleb<stream_writer_t, int32_t>(writer, register_default))
    return false;

  for (const auto& [ name, index ] : register_index_map) {
    std::string register_name = name.substr(name.find_last_of(".") + 1);
    uint32_t register_value;
    
    try {
      register_value = register_name_map.at(register_name);
    } catch(std::out_of_range &oor) {
      err.error("Invalid register name %s\n", register_name);
      return false;
    }

    if (!write_uleb<stream_writer_t, uint32_t>(writer, register_value))
      return false;
    if (!write_uleb<stream_writer_t, uint32_t>(writer, index))
      return false;
  }

  if (!write_uleb<stream_writer_t, uint32_t>(writer, csr_index_map.size()))
    return false;
  if (!write_uleb<stream_writer_t, int32_t>(writer, csr_default))
    return false;

  for (const auto& [ name, index ] : csr_index_map) {
    std::string csr_name = name.substr(name.find_last_of(".") + 1);
    uint32_t csr_value;
    
    try {
      csr_value = csr_name_map.at(csr_name);
    } catch(std::out_of_range &oor) {
      err.error("Invalid csr name %s\n", csr_name);
      return false;
    }

    if (!write_uleb<stream_writer_t, uint32_t>(writer, csr_value))
      return false;
    if (!write_uleb<stream_writer_t, uint32_t>(writer, index))
      return false;
  }

  if (!write_uleb<stream_writer_t, int32_t>(writer, env_default))
    return false;

  if (!write_uleb<stream_writer_t, uint32_t>(writer, memory_index_map.size()))
    return false;
  for (const auto& [ range, index ] : memory_index_map) {
    if (!write_uleb<stream_writer_t, uint64_t>(writer, range.start))
      return false;
    if (!write_uleb<stream_writer_t, uint64_t>(writer, range.end))
      return false;
    if (!write_uleb<stream_writer_t, uint32_t>(writer, index))
      return false;
  }
  return true;
}

bool policy_engine::write_headers(
  std::list<range_t>& code_ranges,
  std::list<std::pair<range_t, uint8_t>>& data_ranges,
  bool is_64_bit,
  const std::string& tag_filename
) {
  stream_writer_t writer(tag_filename);
  if (!writer)
    return false;

  if (!write_uleb<stream_writer_t, uint8_t>(writer, (uint8_t)is_64_bit))
    return false;

  if (!write_uleb<stream_writer_t, uint32_t>(writer, (uint32_t)code_ranges.size()))
    return false;
  for (const range_t& range : code_ranges) {
    if (!write_uleb<stream_writer_t, uint64_t>(writer, range.start))
      return false;
    if (!write_uleb<stream_writer_t, uint64_t>(writer, range.end))
      return false;
  }

  if (!write_uleb<stream_writer_t, uint32_t>(writer, (uint32_t)data_ranges.size()))
    return false;
  for (const auto& [ range, gran ] : data_ranges) {
    if (!write_uleb<stream_writer_t, uint64_t>(writer, range.start))
      return false;
    if (!write_uleb<stream_writer_t, uint64_t>(writer, range.end))
      return false;
    if (!write_uleb<stream_writer_t, uint64_t>(writer, (uint32_t)gran))
      return false;
  }

  return true;
}

bool policy_engine::load_firmware_tag_file(
  std::list<range_t>& code_ranges,
  std::list<range_t>& data_ranges,
  std::vector<std::shared_ptr<metadata_t>>& metadata_values,
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

  stream_reader_t reader(file_name);
  if (!reader)
    return false;

  if (!read_uleb<stream_reader_t, uint8_t>(reader, is_64_bit))
    return false;

  if (!read_uleb<stream_reader_t, uint32_t>(reader, code_range_count))
    return false;
  for (size_t i = 0; i < code_range_count; i++) {
    range_t range;
    if (!read_uleb<stream_reader_t, uint64_t>(reader, range.start))
      return false;
    if (!read_uleb<stream_reader_t, uint64_t>(reader, range.end))
      return false;
    code_ranges.push_back(range);
  }

  if (!read_uleb<stream_reader_t, uint32_t>(reader, data_range_count))
    return false;
  for (size_t i = 0; i < data_range_count; i++) {
    range_t range;
    if (!read_uleb<stream_reader_t, uint64_t>(reader, range.start))
      return false;
    if (!read_uleb<stream_reader_t, uint64_t>(reader, range.end))
      return false;
    data_ranges.push_back(range);
    size_t tag_granularity;
    if (!read_uleb<stream_reader_t, size_t>(reader, tag_granularity))
      return false;
  }

  if (!read_uleb<stream_reader_t, uint32_t>(reader, metadata_value_count))
    return false;
  for (size_t i = 0; i < metadata_value_count; i++) {
    uint32_t metadata_count;
    if (!read_uleb<stream_reader_t, uint32_t>(reader, metadata_count))
      return false;

    std::shared_ptr<metadata_t> metadata = std::make_shared<metadata_t>();
    for (size_t j = 0; j < metadata_count; j++) {
      meta_t meta;
      if(!read_uleb<stream_reader_t, meta_t>(reader, meta))
        return false;
      metadata->insert(meta);
    }
    metadata_values.push_back(metadata);
  }

  if (!read_uleb<stream_reader_t, uint32_t>(reader, register_index_count))
    return false;
  if (!read_uleb<stream_reader_t, int32_t>(reader, register_default))
    return false;
  for (size_t i = 0; i < register_index_count; i++) {
    std::string register_name;
    uint32_t register_value;
    uint32_t register_meta;

    if (!read_uleb<stream_reader_t, uint32_t>(reader, register_value))
      return false;
    if (!read_uleb<stream_reader_t, uint32_t>(reader, register_meta))
      return false;

    const auto it = std::find_if(register_name_map.begin(), register_name_map.end(), [&](const std::pair<std::string, uint32_t>& e){ return e.second == register_value; });
    if (it == register_name_map.end())
      return false;

    std::pair<std::string, uint32_t> p(it->first, register_meta);
    register_index_map.insert(std::make_pair(it->first, register_meta));
  }

  if (!read_uleb<stream_reader_t, uint32_t>(reader, csr_index_count))
    return false;
  if (!read_uleb<stream_reader_t, int32_t>(reader, csr_default))
    return false;
  for (size_t i = 0; i < csr_index_count; i++) {
    uint32_t csr_value;
    uint32_t csr_meta;
    if (!read_uleb<stream_reader_t, uint32_t>(reader, csr_value))
      return false;
    if (!read_uleb<stream_reader_t, uint32_t>(reader, csr_meta))
      return false;

    const auto it = std::find_if(csr_name_map.begin(), csr_name_map.end(), [&](const std::pair<std::string, uint32_t>& e){ return e.second == csr_value; });
    if (it == csr_name_map.end())
      return false;

    csr_index_map.insert(std::make_pair(it->first, csr_meta));
  }

  if (!read_uleb<stream_reader_t, int32_t>(reader, env_default))
    return false;

  if (!read_uleb<stream_reader_t, uint32_t>(reader, memory_index_count))
    return false;
  for (size_t i = 0; i < memory_index_count; i++) {
    range_t range;
    uint32_t metadata_index;

    if (!read_uleb<stream_reader_t, uint64_t>(reader, range.start))
      return false;
    if (!read_uleb<stream_reader_t, uint64_t>(reader, range.end))
      return false;
    if (!read_uleb<stream_reader_t, uint32_t>(reader, metadata_index))
      return false;
    metadata_index_map.insert(std::make_pair(range, metadata_index));
  }

  return true;
}