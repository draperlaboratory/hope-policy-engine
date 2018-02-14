#include <stdio.h>

#include "tag_file.h"
#include "uleb.h"

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

bool load_tags(metadata_memory_map_t *map, std::string file_name) {
  FILE *fp = fopen(file_name.c_str(), "rb");

  if (!fp)
    return false;

  file_reader_t reader(fp);
  fseek(fp, 0, SEEK_END);
  size_t eof_point = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  
//  while (!feof(fp)) {
  while (eof_point != ftell(fp)) {
    address_t start;
    address_t end;
    meta_set_t ms;
    if (!read_uleb<file_reader_t, uint32_t>(&reader, start)) {
      fclose(fp);
      return false;
    }
    if (!read_uleb<file_reader_t, uint32_t>(&reader, end)) {
      fclose(fp);
      return false;
    }
    for (int i = 0; i < META_SET_WORDS; i++) {
      if (!read_uleb<file_reader_t, uint32_t>(&reader, ms.tags[i])) {
	fclose(fp);
	return false;
      }
    }
    map->add_range(start, end, &ms);
  }
  fclose(fp);
  return true;
}

bool save_tags(metadata_memory_map_t *map, std::string file_name) {
  FILE *fp = fopen(file_name.c_str(), "wb");

  if (!fp)
    return false;
  file_writer_t writer(fp);
  for (auto &e: *map) {
//    printf("writing 0x%x\n", e.first.start);
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.first.start)) {
      fclose(fp);
      return false;
    }
    if (!write_uleb<file_writer_t, uint32_t>(&writer, e.first.end)) {
      fclose(fp);
      return false;
    }
    for (int i = 0; i < META_SET_WORDS; i++) {
      if (!write_uleb<file_writer_t, uint32_t>(&writer, e.second->tags[i])) {
	fclose(fp);
	return false;
      }
    }
  }
  fclose(fp);
  return true;
}
