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

#include <cstdio>
#include <memory>
#include <string>
#include "metadata.h"
#include "policy_meta_set.h"
#include "register_name_map.h"
#include "reporter.h"
#include "tag_file.h"
#include "uleb.h"

using namespace policy_engine;

struct file_reader_t {
  std::FILE* const fp;
  bool read_byte(uint8_t &b) const { return fread(&b, 1, 1, fp) == 1; }
};

struct file_writer_t {
  std::FILE* const fp;
  bool write_byte(uint8_t &b) const { return fwrite(&b, 1, 1, fp) == 1; }
};

bool policy_engine::load_tags(metadata_memory_map_t& map, const std::string& file_name) {
  file_reader_t reader{fopen(file_name.c_str(), "rb")};
  if (!reader.fp)
    return false;

  fseek(reader.fp, 0, SEEK_END);
  size_t eof_point = ftell(reader.fp);
  fseek(reader.fp, 0, SEEK_SET);
  
  while (eof_point != ftell(reader.fp)) {
    uint64_t start, end;
    uint32_t metadata_count;

    if (!read_uleb<file_reader_t, uint64_t>(&reader, start)) {
      fclose(reader.fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint64_t>(&reader, end)) {
      fclose(reader.fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_count)) {
      fclose(reader.fp);
      return false;
    }
    std::shared_ptr<metadata_t> metadata = std::make_shared<metadata_t>();
    for (uint32_t i = 0; i < metadata_count; i++) {
      meta_t meta;
      if (!read_uleb<file_reader_t, meta_t>(&reader, meta)) {
        fclose(reader.fp);
        return false;
      }
      metadata->insert(meta);
    }
    map.add_range(start, end, metadata);
  }
  fclose(reader.fp);
  return true;
}

bool policy_engine::save_tags(metadata_memory_map_t& map, std::string file_name) {
  file_writer_t writer{fopen(file_name.c_str(), "wb")};
  if (!writer.fp)
    return false;

  for (const auto& [ range, md ] : map) {
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.start)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.end)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, md->size())) {
      fclose(writer.fp);
      return false;
    }
    for (const meta_t& meta : *md) {
      if (!write_uleb<file_writer_t, meta_t>(&writer, meta)) {
        fclose(writer.fp);
        return false;
      }
    }
  }
  fclose(writer.fp);
  return true;
}

bool policy_engine::save_tag_indexes(std::vector<std::shared_ptr<metadata_t>> &metadata_values,
                                     metadata_index_map_t<metadata_memory_map_t, range_t> &memory_index_map,
                                     metadata_index_map_t<metadata_register_map_t, std::string> &register_index_map,
                                     metadata_index_map_t<metadata_register_map_t, std::string> &csr_index_map,
                                     int32_t register_default, int32_t csr_default, int32_t env_default,
                                     std::string file_name,
                                     reporter_t& err) {
  file_writer_t writer{fopen(file_name.c_str(), "wb")};
  if (!writer.fp)
    return false;

  if (!write_uleb<file_writer_t, uint32_t>(&writer, metadata_values.size())) {
    fclose(writer.fp);
    return false;
  }

  for (const std::shared_ptr<metadata_t>& v : metadata_values) {
    if (!write_uleb<file_writer_t, uint32_t>(&writer, v->size())) {
      fclose(writer.fp);
      return false;
    }
    for (const meta_t& m : *v) {
      if (!write_uleb<file_writer_t, meta_t>(&writer, m)) {
        fclose(writer.fp);
        return false;
      }
    }
  }

  if (!write_uleb<file_writer_t, uint32_t>(&writer, register_index_map.size())) {
    fclose(writer.fp);
    return false;
  }
  if (!write_uleb<file_writer_t, int32_t>(&writer, register_default)) {
    fclose(writer.fp);
    return false;
  }

  for (const auto& [ name, index ] : register_index_map) {
    std::string register_name = name.substr(name.find_last_of(".") + 1);
    uint32_t register_value;
    
    try {
      register_value = register_name_map.at(register_name);
    } catch(std::out_of_range &oor) {
      err.error("Invalid register name %s\n", register_name);
      return false;
    }

    if (!write_uleb<file_writer_t, uint32_t>(&writer, register_value)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, index)) {
      fclose(writer.fp);
      return false;
    }
  }

  if (!write_uleb<file_writer_t, uint32_t>(&writer, csr_index_map.size())) {
    fclose(writer.fp);
    return false;
  }
  if (!write_uleb<file_writer_t, int32_t>(&writer, csr_default)) {
    fclose(writer.fp);
    return false;
  }

  for (const auto& [ name, index ] : csr_index_map) {
    std::string csr_name = name.substr(name.find_last_of(".") + 1);
    uint32_t csr_value;
    
    try {
      csr_value = csr_name_map.at(csr_name);
    } catch(std::out_of_range &oor) {
      err.error("Invalid csr name %s\n", csr_name);
      return false;
    }

    if (!write_uleb<file_writer_t, uint32_t>(&writer, csr_value)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, index)) {
      fclose(writer.fp);
      return false;
    }
  }

  if (!write_uleb<file_writer_t, int32_t>(&writer, env_default)) {
    fclose(writer.fp);
    return false;
  }

  if (!write_uleb<file_writer_t, uint32_t>(&writer, memory_index_map.size())) {
    fclose(writer.fp);
    return false;
  }

  for (const auto& [ range, index ] : memory_index_map) {
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.start)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.end)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, index)) {
      fclose(writer.fp);
      return false;
    }
  }
  fclose(writer.fp);
  return true;
}

bool policy_engine::write_headers(std::list<range_t> &code_ranges,
                                  std::list<std::pair<range_t, uint8_t>> &data_ranges,
                                  bool is_64_bit, std::string tag_filename) {
  std::FILE* in_fp = fopen(tag_filename.c_str(), "rb");
  if (in_fp == NULL) {
    return false;
  }

  fseek(in_fp, 0L, SEEK_END);
  size_t in_size = ftell(in_fp);
  fseek(in_fp, 0L, SEEK_SET);

  uint8_t in_buffer[in_size];
  if (fread(in_buffer, 1, in_size, in_fp) != in_size) {
    fclose(in_fp);
    return false;
  }

  fclose(in_fp);

  file_writer_t writer{fopen(tag_filename.c_str(), "wb")};
  if (writer.fp == NULL) {
    return false;
  }

  if (!write_uleb<file_writer_t, uint8_t>(&writer, (uint8_t)is_64_bit)) {
    fclose(writer.fp);
    return false;
  }

  if (!write_uleb<file_writer_t, uint32_t>(&writer, (uint32_t)code_ranges.size())) {
    fclose(writer.fp);
    return false;
  }
  for (const range_t& range : code_ranges) {
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.start)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.end)) {
      fclose(writer.fp);
      return false;
    }
  }

  if (!write_uleb<file_writer_t, uint32_t>(&writer, (uint32_t)data_ranges.size())) {
    fclose(writer.fp);
    return false;
  }
  for (const auto& [ range, gran ] : data_ranges) {
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.start)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint64_t>(&writer, range.end)) {
      fclose(writer.fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint64_t>(&writer, (uint32_t)gran)) {
      fclose(writer.fp);
      return false;
    }
  }

  if (fwrite(in_buffer, 1, in_size, writer.fp) != in_size) {
    fclose(writer.fp);
    return false;
  }

  fclose(writer.fp);
  return true;
}

bool policy_engine::load_firmware_tag_file(std::list<range_t> &code_ranges,
                                           std::list<range_t> &data_ranges,
                                           std::vector<std::shared_ptr<metadata_t>> &metadata_values,
                                           metadata_index_map_t<metadata_memory_map_t, range_t> &metadata_index_map,
                                           metadata_index_map_t<metadata_register_map_t, std::string> &register_index_map,
                                           metadata_index_map_t<metadata_register_map_t, std::string> &csr_index_map,
                                           int32_t &register_default, int32_t &csr_default, int32_t &env_default,
                                           std::string file_name,
                                           reporter_t& err) {
  uint8_t is_64_bit;
  uint32_t code_range_count;
  uint32_t data_range_count;
  uint32_t metadata_value_count;
  uint32_t memory_index_count;
  uint32_t register_index_count;
  uint32_t csr_index_count;
  file_reader_t reader{fopen(file_name.c_str(), "rb")};
  if (!reader.fp)
    return false;

  fseek(reader.fp, 0, SEEK_END);
  size_t eof_point = ftell(reader.fp);
  fseek(reader.fp, 0, SEEK_SET);

  if (!read_uleb<file_reader_t, uint8_t>(&reader, is_64_bit)) {
    fclose(reader.fp);
    return false;
  }

  if (!read_uleb<file_reader_t, uint32_t>(&reader, code_range_count)) {
    fclose(reader.fp);
    return false;
  }
  for (size_t i = 0; i < code_range_count; i++) {
    range_t range;
    if (!read_uleb<file_reader_t, uint64_t>(&reader, range.start)) {
      fclose(reader.fp);
      return false;
    }

    if (!read_uleb<file_reader_t, uint64_t>(&reader, range.end)) {
      fclose(reader.fp);
      return false;
    }
    code_ranges.push_back(range);
  }

  if (!read_uleb<file_reader_t, uint32_t>(&reader, data_range_count)) {
    fclose(reader.fp);
    return false;
  }
  for (size_t i = 0; i < data_range_count; i++) {
    range_t range;
    if (!read_uleb<file_reader_t, uint64_t>(&reader, range.start)) {
      fclose(reader.fp);
      return false;
    }

    if (!read_uleb<file_reader_t, uint64_t>(&reader, range.end)) {
      fclose(reader.fp);
      return false;
    }
    data_ranges.push_back(range);
    size_t tag_granularity;
    if (!read_uleb<file_reader_t, size_t>(&reader, tag_granularity)) {
      fclose(reader.fp);
      return false;
    }
  }

  if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_value_count)) {
    fclose(reader.fp);
    return false;
  }

  for (size_t i = 0; i < metadata_value_count; i++) {
    uint32_t metadata_count;

    if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_count)) {
      fclose(reader.fp);
      return false;
    }

    std::shared_ptr<metadata_t> metadata = std::make_shared<metadata_t>();
    for (size_t j = 0; j < metadata_count; j++) {
      meta_t meta;
      if(!read_uleb<file_reader_t, meta_t>(&reader, meta)) {
        fclose(reader.fp);
        return false;
      }
      metadata->insert(meta);
    }
    metadata_values.push_back(metadata);
  }

  if (!read_uleb<file_reader_t, uint32_t>(&reader, register_index_count)) {
    fclose(reader.fp);
    return false;
  }
  if (!read_uleb<file_reader_t, int32_t>(&reader, register_default)) {
    fclose(reader.fp);
    return false;
  }

  for (size_t i = 0; i < register_index_count; i++) {
    std::string register_name;
    uint32_t register_value;
    uint32_t register_meta;

    if (!read_uleb<file_reader_t, uint32_t>(&reader, register_value)) {
      fclose(reader.fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, register_meta)) {
      fclose(reader.fp);
      err.error("Failed here\n");
      return false;
    }

    for (const auto& [ name, index ] : register_name_map) {
      if (register_value == index) {
        register_name = name;
        break;
      }
    }

    if (register_name.empty()) {
      return false;
    }

    std::pair<std::string, uint32_t> p(register_name, register_meta);
    register_index_map.insert(p);
  }

  if (!read_uleb<file_reader_t, uint32_t>(&reader, csr_index_count)) {
    fclose(reader.fp);
    return false;
  }
  if (!read_uleb<file_reader_t, int32_t>(&reader, csr_default)) {
    fclose(reader.fp);
    return false;
  }

  for (size_t i = 0; i < csr_index_count; i++) {
    std::string csr_name;
    uint32_t csr_value;
    uint32_t csr_meta;

    if (!read_uleb<file_reader_t, uint32_t>(&reader, csr_value)) {
      fclose(reader.fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, csr_meta)) {
      fclose(reader.fp);
      return false;
    }

    for (const auto& [ name, index ] : csr_name_map) {
      if (csr_value == index) {
        csr_name = name;
        break;
      }
    }

    if (csr_name.empty()) {
      return false;
    }

    std::pair<std::string, uint32_t> p(csr_name, csr_meta);
    csr_index_map.insert(p);
  }

  if (!read_uleb<file_reader_t, int32_t>(&reader, env_default)) {
    fclose(reader.fp);
    return false;
  }

  if (!read_uleb<file_reader_t, uint32_t>(&reader, memory_index_count)) {
    fclose(reader.fp);
    return false;
  }

  for (size_t i = 0; i < memory_index_count; i++) {
    range_t range;
    uint32_t metadata_index;

    if (!read_uleb<file_reader_t, uint64_t>(&reader, range.start)) {
      fclose(reader.fp);
      return false;
    }

    if (!read_uleb<file_reader_t, uint64_t>(&reader, range.end)) {
      fclose(reader.fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_index)) {
      fclose(reader.fp);
      return false;
    }

    std::pair<range_t, uint32_t> p(range, metadata_index);
    metadata_index_map.insert(p);
  }

  return true;
}