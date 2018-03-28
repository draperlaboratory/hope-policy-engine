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
#include "metadata_memory_map.h"
#include "metadata_cache.h"
#include "metadata_factory.h"
#include "validator_exception.h"

using namespace policy_engine;

metadata_cache_t md_cache;
metadata_factory_t *md_factory;

extern void init_metadata_renderer(metadata_factory_t *md_factory);

void init(const char *policy_dir) {
  try {
    md_factory = new metadata_factory_t(policy_dir);
    init_metadata_renderer(md_factory);
  } catch (exception_t &e) {
    printf("exception: %s\n", e.what());
  }
}

bool apply_tag(metadata_memory_map_t *map, address_t start, address_t end, const char *tag_name) {
  metadata_t const *md = md_factory->lookup_metadata(tag_name);
  if (!md)
    return false;
  map->add_range(start, end, md);
  return true;
}

#include <fstream>
#include <sstream>
#include <string>
bool load_range_file(metadata_memory_map_t *map, std::string file_name) {
  int lineno = 1;
  bool res = true;
  try {
    std::ifstream infile(file_name);
    std::string line;
    while (std::getline(infile, line)) {
      std::istringstream iss(line);
//      printf("Line = '%s'\n", line.c_str());
      std::vector<std::string> tokens {std::istream_iterator<std::string>{iss},
	  std::istream_iterator<std::string>{}};
      if (tokens.size() != 3) {
	fprintf(stderr, "%s: %d: bad format - wrong number of items\n", file_name.c_str(), lineno);
	res = false;
      } else {
	address_t start;
	address_t end;
	start = strtol(tokens[0].c_str(), 0, 16);
	end = strtol(tokens[1].c_str(), 0, 16);
//	printf("applying tag to 0x%x, 0x%x ... ", start, end);
	if (!apply_tag(map, start, end, tokens[2].c_str())) {
	  fprintf(stderr, "%s: %d: could not find tag %s\n", file_name.c_str(), lineno, tokens[2].c_str());
	  res = false;
	} else {
//	  printf("done\n");
	}
      }
      lineno++;
    }
  } catch (...) {
    fprintf(stderr, "error loading %s\n", file_name.c_str());
    return false;
  }
  return res;
}

void usage() {
  printf("usage: md_range <policy_dir> <range_file> <tag_file>\n");
}

int main(int argc, char **argv) {
  const char *policy_dir;
  const char *range_file_name;
  const char *file_name;

  if (argc != 4) {
    usage();
    return 0;
  }

  policy_dir = argv[1];
  range_file_name = argv[2];
  file_name = argv[3];

  init(policy_dir);

  metadata_memory_map_t map(&md_cache);

  if (!load_range_file(&map, range_file_name))
    return 1;

  if (!save_tags(&map, file_name)) {
    printf("failed write of tag file\n");
    return 1;
  }

  return 0;
}
