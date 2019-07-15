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
#include "tag_file.h"
#include "elf_utils.h"
#include "basic_elf_io.h"
#include "metadata_index_map.h"

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

void usage() {
  printf("usage: md_firmware_test <tag_file> <num_entries (16)>\n");
}

int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *tag_filename;
  std::list<range_t> code_ranges;
  std::list<range_t> data_ranges;
  std::vector<const metadata_t *> metadata_values;
  metadata_index_map_t metadata_index_map;
  size_t num_entries = 16;

  if(argc < 2) {
    usage();
    return 1;
  }

  if(argc == 3) {
    num_entries = strtoul(argv[2], NULL, 0);
  }

  tag_filename = argv[1];

  if(load_firmware_tag_file(code_ranges, data_ranges, metadata_values,
        metadata_index_map, std::string(tag_filename)) == false) {
    err.error("Failed to load firmware tag file\n");
    return 1;
  }

  printf("Code ranges:\n");
  for(auto &r : code_ranges) {
    printf("{ 0x%08x - 0x%08x }\n", r.start, r.end);
  }

  printf("\nData ranges:\n");
  for(auto &r : data_ranges) {
    printf("{ 0x%08x - 0x%08x }\n", r.start, r.end);
  }

  printf("\nMetadata values:\n");
  for(size_t i = 0; i < metadata_values.size(); i++) {
    printf("%lu: { ", i);
    for(const auto &m : *metadata_values[i]) {
      printf("%lx ", m);
    }
    printf("}\n");
  }

  printf("\nTag entries (showing %lu of %lu):\n",
      num_entries, metadata_index_map.size());
  size_t entry_index = 0;
  for(auto &it : metadata_index_map) {
    printf("{ 0x%08x - 0x%08x }: %x\n", it.first.start, it.first.end, it.second);

    entry_index++;
    if(entry_index == num_entries) {
      break;
    }
  }

  return 0;
}
