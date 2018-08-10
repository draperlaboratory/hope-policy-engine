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

  int alloc_size = 0;
  
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

    printf("LT: (0x%x, 0x%x): %d meta_t (size %d)\n", start, end, metadata_count, metadata->size());
    alloc_size += metadata->size();
    map->add_range(start, end, metadata);
  }

  printf("TOTAL SIZE OF ALLOCED METADATA = %d\n", alloc_size);
  fclose(fp);
  return true;
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

printf("ST: (0x%x, 0x%x): %d tags\n", e.first.start, e.first.end, e.second->size());
    for (auto &m: e.second->pull_metadata()) {

      printf("writing tag 0x%d\n", m);
      
      if (!write_uleb<file_writer_t, meta_t>(&writer, m)) {
	fclose(fp);
	return false;
      }
    }
  }
  fclose(fp);
  return true;
}
