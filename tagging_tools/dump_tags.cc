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

#include <stdio.h>

#include "basic_elf_io.h"
#include "tag_file.h"
#include "uleb.h"

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

struct file_reader_t {
  FILE *fp;
  file_reader_t(FILE *fp) : fp(fp) { }
  bool read_byte(uint8_t &b) {
    return fread(&b, 1, 1, fp) == 1;
  }
};

void usage() {
  printf("usage: dump_tags <tag_file>\n");
}

bool dump_tags(std::string file_name) {
  FILE *fp = fopen(file_name.c_str(), "rb");
  int i = 0;

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

    if (!read_uleb<file_reader_t, address_t>(&reader, start)) {
      fclose(fp);
      return false;
    }

    if (!read_uleb<file_reader_t, address_t>(&reader, end)) {
      fclose(fp);
      return false;
    }

    if (!read_uleb<file_reader_t, uint32_t>(&reader, metadata_count)) {
      fclose(fp);
      return false;
    }

#ifdef RV64_VALIDATOR
    printf("Entry %d, 0x%016lx - 0x%016lx (%d)\n", i++,
           start, end, metadata_count);
#else
    printf("Entry %d, 0x%08x - 0x%08x (%d)\n", i++,
           start, end, metadata_count);
#endif
    printf("\tMetadata List:  ");

    for (uint32_t i = 0; i < metadata_count; i++) {
      meta_t meta;

      if (!read_uleb<file_reader_t, meta_t>(&reader, meta)) {
         fclose(fp);
         return false;
      }

      printf("%016lx, ", meta);
    }

    printf("end.\n");
  }

  fclose(fp);

  return true;
}

int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *tag_filename;

  if(argc < 2) {
    usage();
    return 1;
  }

  // Retrieve memory metadata from tag file
  tag_filename = argv[1];
  if(dump_tags(tag_filename) == false) {
    err.error("Failed to dump tags\n");
    return 1;
  }

  return 0;
}
