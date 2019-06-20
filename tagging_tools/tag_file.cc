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
//    printf("(0x%x, 0x%x): %d meta_t\n", start, end, metadata_count);
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

bool policy_engine::write_headers(std::pair<uintptr_t, uintptr_t> code_range,
                                   std::pair<uintptr_t, uintptr_t> data_range,
                                   bool is_64_bit, std::string file_name) {
  FILE *in_fp, *out_fp;
  size_t in_size;
  uint8_t *in_buffer;

  in_fp = fopen(file_name.c_str(), "rb");
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

  out_fp = fopen(file_name.c_str(), "wb");
  if(out_fp == NULL) {
    return false;
  }

  file_writer_t writer(out_fp);

  if(!write_uleb<file_writer_t, uint8_t>(&writer, (uint8_t)is_64_bit)) {
    fclose(out_fp);
    return false;
  }

  if(!write_uleb<file_writer_t, uintptr_t>(&writer, code_range.first)) {
    fclose(out_fp);
    return false;
  }
  if(!write_uleb<file_writer_t, uintptr_t>(&writer, code_range.second)) {
    fclose(out_fp);
    return false;
  }

  if(!write_uleb<file_writer_t, uintptr_t>(&writer, data_range.first)) {
    fclose(out_fp);
    return false;
  }
  if(!write_uleb<file_writer_t, uintptr_t>(&writer, data_range.second)) {
    fclose(out_fp);
    return false;
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
