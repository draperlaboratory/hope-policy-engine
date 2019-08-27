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
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "tag_file.h"
#include "uleb.h"

using namespace policy_engine;

struct file_reader_t {
  FILE *fp;
  file_reader_t(FILE *fp) : fp(fp) { }
  bool read_byte(uint8_t &b) {
    return fread(&b, 1, 1, fp) == 1;
  }
};

struct file_writer_t {
  FILE *fp;
  file_writer_t(FILE *fp) : fp(fp) { }
  bool write_byte(uint8_t &b) {
    return fwrite(&b, 1, 1, fp) == 1;
  }
};

bool policy_engine::load_tags(metadata_memory_map_t *map, std::string file_name) {
  FILE *fp = fopen(file_name.c_str(), "rb");

  if (!fp)
    return false;

  file_reader_t reader(fp);
  fseek(fp, 0, SEEK_END);
  size_t eof_point = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  
  while (eof_point != ftell(fp)) {
    address_t start;
    address_t end;
    uint32_t metadata_count;

    if (!read_uleb<file_reader_t, uint32_t>(&reader, start)) {
      fclose(fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, end)) {
      fclose(fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_count)) {
      fclose(fp);
      return false;
    }
    //printf("tagfile read (0x%x, 0x%x): %d meta_t\n", start, end, metadata_count);
    metadata_t *metadata = new metadata_t();
    for (uint32_t i = 0; i < metadata_count; i++) {
      meta_t meta;
      if (!read_uleb<file_reader_t, meta_t>(&reader, meta)) {
	fclose(fp);
	delete metadata;
	return false;
      }
      metadata->insert(meta);
    }
    map->add_range(start, end, metadata);
  }
  fclose(fp);
  return true;
}

// Creates a map that maps addresses to vectors of field values to set on those addresses.
// Map is created based on content of taginfo.args file that is loaded automatically.
arg_val_map_t * policy_engine::load_tag_args(metadata_memory_map_t *map, std::string file_name) {
  
  printf("Loading tag arguments file %s\n", file_name.c_str());

  arg_val_map_t * tag_arg_map = new std::map<uint32_t, std::vector<uint32_t>*>();
  
  // Read taginfo.args file. Current format is just ASCII.
  try {
    
    std::string line;    
    std::ifstream infile(file_name);
    if (infile.fail()){
      printf("Warning: did not find taginfo.args file. No argument set.\n");
      return tag_arg_map;
    }
    
    while (std::getline(infile, line)) {

      // Cut into tokens
      std::istringstream iss(line);
      std::vector<std::string> tokens {std::istream_iterator<std::string>{iss},
	  std::istream_iterator<std::string>{}};
      
      //printf("Line = '%s', numtokens=%lu\n", line.c_str(), tokens.size());
      if (tokens.size() < 2){
	printf("Bad line, less than two tokens: %s\n", line.c_str());
	continue;
      }
      
      // Extract start and end
      address_t start;
      address_t end;
      start = strtol(tokens[0].c_str(), 0, 16);
      end = strtol(tokens[1].c_str(), 0, 16);

      // Make vector out of argument ints
      std::vector<uint32_t> * argument_values = new std::vector<uint32_t>();
      for (int i = 2; i < tokens.size(); i++){
	uint32_t arg_val = strtol(tokens[i].c_str(), 0, 10);
 	argument_values -> push_back(arg_val);
      }

      // Add to map on each address
      uint32_t current;
      for (current = start; current < end; current +=4 ){
	//printf("Adding to tag arg map on addr %d\n", current);
	tag_arg_map -> insert(std::pair<uint32_t, std::vector<uint32_t>*>(current, argument_values));
      }
    }
  } catch (...) {
    fprintf(stderr, "error loading %s\n", file_name.c_str());
    return NULL;
  }  
  return tag_arg_map;
}

bool policy_engine::save_tags(metadata_memory_map_t *map, std::string file_name) {
  FILE *fp = fopen(file_name.c_str(), "wb");

  if (!fp)
    return false;
  file_writer_t writer(fp);
  for (auto &e: *map) {
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.first.start)) {
      fclose(fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.first.end)) {
      fclose(fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.second->size())) {
      fclose(fp);
      return false;
    }
    for (auto &m: *e.second) {
      if (!write_uleb<file_writer_t, meta_t>(&writer, m)) {
	fclose(fp);
	return false;
      }
    }
  }
  fclose(fp);
  return true;
}

bool policy_engine::save_tag_indexes(std::vector<const metadata_t *> &metadata_values,
                                     std::map<range_t, uint32_t, range_compare> &index_map,
                                     std::string file_name) {
  FILE *fp = fopen(file_name.c_str(), "wb");

  if (!fp)
    return false;
  file_writer_t writer(fp);

  if(!write_uleb<file_writer_t, uint32_t>(&writer, metadata_values.size())) {
    fclose(fp);
    return false;
  }

  for (auto &v: metadata_values) {
    if (!write_uleb<file_writer_t, uint32_t>(&writer, v->size())) {
      fclose(fp);
      return false;
    }
    for (auto &m: *v) {
      if (!write_uleb<file_writer_t, meta_t>(&writer, m)) {
        fclose(fp);
        return false;
      }
    }
  }

  if(!write_uleb<file_writer_t, uint32_t>(&writer, index_map.size())) {
    fclose(fp);
    return false;
  }

  for (auto &e: index_map) {
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.first.start)) {
      fclose(fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.first.end)) {
      fclose(fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.second)) {
      fclose(fp);
      return false;
    }
  }
  fclose(fp);
  return true;
}

bool policy_engine::write_headers(std::list<range_t> &code_ranges,
                                  std::list<range_t> &data_ranges,
                                  bool is_64_bit, std::string tag_filename) {
  FILE *in_fp, *out_fp;
  size_t in_size;
  uint8_t *in_buffer;

  in_fp = fopen(tag_filename.c_str(), "rb");
  if(in_fp == NULL) {
    return false;
  }

  fseek(in_fp, 0L, SEEK_END);
  in_size = ftell(in_fp);
  fseek(in_fp, 0L, SEEK_SET);

  in_buffer = (uint8_t *)malloc(in_size);
  if(in_buffer == NULL) {
    fclose(in_fp);
    return false;
  }

  if (fread(in_buffer, 1, in_size, in_fp) != in_size) {
    free(in_buffer);
    fclose(in_fp);
    return false;
  }

  fclose(in_fp);

  out_fp = fopen(tag_filename.c_str(), "wb");
  if(out_fp == NULL) {
    return false;
  }

  file_writer_t writer(out_fp);

  if(!write_uleb<file_writer_t, uint8_t>(&writer, (uint8_t)is_64_bit)) {
    fclose(out_fp);
    return false;
  }

  if(!write_uleb<file_writer_t, uint32_t>(&writer, (uint32_t)code_ranges.size())) {
    fclose(out_fp);
    return false;
  }
  for(const auto &it : code_ranges) {
    if(!write_uleb<file_writer_t, address_t>(&writer, it.start)) {
      fclose(out_fp);
      return false;
    }
    if(!write_uleb<file_writer_t, address_t>(&writer, it.end)) {
      fclose(out_fp);
      return false;
    }
  }

  if(!write_uleb<file_writer_t, uint32_t>(&writer, (uint32_t)data_ranges.size())) {
    fclose(out_fp);
    return false;
  }
  for(const auto &it : data_ranges) {
    if(!write_uleb<file_writer_t, address_t>(&writer, it.start)) {
      fclose(out_fp);
      return false;
    }
    if(!write_uleb<file_writer_t, address_t>(&writer, it.end)) {
      fclose(out_fp);
      return false;
    }
  }

  if(fwrite(in_buffer, 1, in_size, out_fp) != in_size) {
    free(in_buffer);
    fclose(out_fp);
    return false;
  }

  fclose(out_fp);
  free(in_buffer);

  return true;
}

bool policy_engine::load_firmware_tag_file(std::list<range_t> &code_ranges,
                                           std::list<range_t> &data_ranges,
                                           std::vector<const metadata_t *> &metadata_values,
                                           metadata_index_map_t &metadata_index_map,
                                           std::string file_name) {
  uint8_t is_64_bit;
  uint32_t code_range_count;
  uint32_t data_range_count;
  uint32_t metadata_value_count;
  uint32_t metadata_index_count;
  FILE *fp = fopen(file_name.c_str(), "rb");

  if(!fp)
    return false;

  file_reader_t reader(fp);
  fseek(fp, 0, SEEK_END);
  size_t eof_point = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if(!read_uleb<file_reader_t, uint8_t>(&reader, is_64_bit)) {
    fclose(fp);
    return false;
  }

  if(!read_uleb<file_reader_t, uint32_t>(&reader, code_range_count)) {
    fclose(fp);
    return false;
  }
  for(size_t i = 0; i < code_range_count; i++) {
    range_t range;
    if(!read_uleb<file_reader_t, address_t>(&reader, range.start)) {
      fclose(fp);
      return false;
    }

    if(!read_uleb<file_reader_t, address_t>(&reader, range.end)) {
      fclose(fp);
      return false;
    }
    code_ranges.push_back(range);
  }

  if(!read_uleb<file_reader_t, uint32_t>(&reader, data_range_count)) {
    fclose(fp);
    return false;
  }
  for(size_t i = 0; i < data_range_count; i++) {
    range_t range;
    if(!read_uleb<file_reader_t, address_t>(&reader, range.start)) {
      fclose(fp);
      return false;
    }

    if(!read_uleb<file_reader_t, address_t>(&reader, range.end)) {
      fclose(fp);
      return false;
    }
    data_ranges.push_back(range);
  }

  if(!read_uleb<file_reader_t, uint32_t>(&reader, metadata_value_count)) {
    fclose(fp);
    return false;
  }

  for(size_t i = 0; i < metadata_value_count; i++) {
    uint32_t metadata_count;

    if(!read_uleb<file_reader_t, uint32_t>(&reader, metadata_count)) {
      fclose(fp);
      return false;
    }

    metadata_t *metadata = new metadata_t();
    for(size_t j = 0; j < metadata_count; j++) {
      meta_t meta;
      if(!read_uleb<file_reader_t, meta_t>(&reader, meta)) {
        fclose(fp);
        delete metadata;
        return false;
      }
      metadata->insert(meta);
    }
    metadata_values.push_back(metadata);
  }

  if(!read_uleb<file_reader_t, uint32_t>(&reader, metadata_index_count)) {
    fclose(fp);
    return false;
  }

  for(size_t i = 0; i < metadata_index_count; i++) {
    range_t range;
    uint32_t metadata_index;

    if (!read_uleb<file_reader_t, uint32_t>(&reader, range.start)) {
      fclose(fp);
      return false;
    }

    if (!read_uleb<file_reader_t, uint32_t>(&reader, range.end)) {
      fclose(fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_index)) {
      fclose(fp);
      return false;
    }

    std::pair<range_t, uint32_t> p(range, metadata_index);
    metadata_index_map.insert(p);
  }

  return true;
}
