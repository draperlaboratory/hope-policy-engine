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

bool extract_metadata_values(std::string file_name,
                             std::vector<const metadata_t *> &metadata_values,
                             metadata_index_map_t &metadata_index_map,
                             reporter_t *err) {
  metadata_memory_map_t metadata_memory_map; 

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

  return true;
}

void read_metadata_value(std::string input, metadata_t *metadata) {
  std::string temp;
  std::string tag_value;
  size_t start = 0;
  size_t end;
  meta_t meta;

  end = input.find(",");
  while(end != std::string::npos) {
    meta = (meta_t) stoi(input.substr(start, end - start));
    metadata->insert(meta);
    start = end + 1;
    end = input.find(",", start);
  }

  meta = (meta_t) stoi(input.substr(start, end - start));
  metadata->insert(meta);
}

size_t add_metadata_value(metadata_t *metadata, std::vector<const metadata_t *> &metadata_values) {
  for(size_t i = 0; i < metadata_values.size(); i++) {
    if (*metadata == *metadata_values[i]) {
      delete metadata;
      return i;
    }
  }

  metadata_values.push_back(metadata);
  return metadata_values.size() - 1;
}

bool add_soc_ranges(std::string file_name,
                    std::vector<const metadata_t *> &metadata_values,
                    metadata_index_map_t &metadata_index_map,
                    reporter_t *err) {
  int lineno = 1;
  bool res = true;

  try {
    std::ifstream infile(file_name);
    std::string line;
    while (std::getline(infile, line)) {
      std::istringstream iss(line);
      std::vector<std::string> tokens {std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>{}};
      if (tokens.size() != 3) {
        fprintf(stderr, "%s: %d: bad format - wrong number of items\n", file_name.c_str(), lineno);
        res = false;
      } else {
        range_t range;
        std::string tag;
        metadata_t *metadata = new metadata_t();
        size_t index;

        range.start = strtol(tokens[0].c_str(), 0, 16);
        range.end = strtol(tokens[1].c_str(), 0, 16);
        tag = tokens[2];

        read_metadata_value(tag, metadata);
        index = add_metadata_value(metadata, metadata_values);

        std::pair<range_t, uint32_t> p(range, index);
        metadata_index_map.insert(p);
      }
      lineno++;
    }
  } catch (...) {
    fprintf(stderr, "error loading %s\n", file_name.c_str());
    return false;
  }
}

void usage() {
  printf("usage: md_index <tag_file> <soc_file>\n");
}

int main(int argc, char **argv) {
  stdio_reporter_t err;
  const char *tag_filename;
  const char *soc_range_filename;
  std::vector<const metadata_t *> metadata_values;
  metadata_index_map_t metadata_index_map;

  if(argc < 3) {
    usage();
    return 1;
  }

  tag_filename = argv[1];
  soc_range_filename = argv[2];

  if(extract_metadata_values(tag_filename, metadata_values, metadata_index_map, &err) == false) {
    printf("Failed to extract metadata values\n");
    return 1;
  }

  add_soc_ranges(soc_range_filename, metadata_values, metadata_index_map, &err);

  if(save_tag_indexes(metadata_values, metadata_index_map, tag_filename) == false) {
    err.error("Failed to save indexes to tag file\n");
    return 1;
  }

  return 0;
}
