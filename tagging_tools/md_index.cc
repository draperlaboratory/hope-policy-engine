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

bool extract_metadata_values(std::string file_name, reporter_t *err) {
  metadata_memory_map_t metadata_memory_map; 
  std::vector<const metadata_t *> metadata_values;
  metadata_index_map_t metadata_index_map;

  if(load_tags(&metadata_memory_map, file_name) == false) {
    err->error("Failed to load tags\n");
    return false;
  }

  for(const auto &it : metadata_memory_map) {
    auto value_iter = std::find(metadata_values.begin(), metadata_values.end(), it.second);
    uint32_t value_index;
    if(value_iter == metadata_values.end()) {
      metadata_values.push_back(it.second);
      value_index = (metadata_values.end() - metadata_values.begin()) - 1;
    }
    else {
      value_index = value_iter - metadata_values.begin();
    }

    std::pair<range_t, uint32_t> p(it.first, value_index);
    metadata_index_map.insert(p);
  }

  printf("Metadata entries:\n");
  for(size_t i = 0; i < metadata_values.size(); i++) {
    printf("%lu: { ", i);
    for(const auto &m : *metadata_values[i]) {
      printf("%lx ", m);
    }
    printf("}\n");
  }

  if(save_tag_indexes(metadata_values, metadata_index_map, file_name) == false) {
    err->error("Failed to save indexes to tag file\n");
    return false;
  }

  return true;
}

void usage() {
  printf("usage: md_index <tag_file>\n");
}

int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *tag_filename;

  if(argc < 2) {
    usage();
    return 1;
  }

  tag_filename = argv[1];

  if(extract_metadata_values(tag_filename, &err) == false) {
    printf("Failed to extract metadata values\n");
    return 1;
  }

  return 0;
}
