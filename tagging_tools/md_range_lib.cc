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
#include <fstream>
#include <sstream>
#include <string>
#include "metadata_cache.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "tag_file.h"
#include "validator_exception.h"

namespace policy_engine {

metadata_factory_t* md_factory;

bool apply_tag(metadata_memory_map_t *map, address_t start, address_t end, const char *tag_name) {
  metadata_t const *md = md_factory->lookup_metadata(tag_name);
  if (!md)
    return false;
  map->add_range(start, end, md);
  return true;
}

bool load_range_file(metadata_memory_map_t *map, std::string file_name) {
  int lineno = 1;
  bool res = true;
  try {
    std::ifstream infile(file_name);
    std::string line;
    while (std::getline(infile, line)) {
      std::istringstream iss(line);
      std::vector<std::string> tokens {std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
      if (tokens.size() != 3) {
        std::fprintf(stderr, "%s: %d: bad format - wrong number of items\n", file_name.c_str(), lineno);
        res = false;
      } else {
        address_t start;
        address_t end;
        start = strtoul(tokens[0].c_str(), 0, 16);
        end = strtoul(tokens[1].c_str(), 0, 16);
        if (!apply_tag(map, start, end, tokens[2].c_str())) {
          std::fprintf(stderr, "%s: %d: could not find tag %s\n", file_name.c_str(), lineno, tokens[2].c_str());
          res = false;
        } else {
        }
      }
      lineno++;
    }
  } catch (...) {
    std::fprintf(stderr, "error loading %s\n", file_name.c_str());
    return false;
  }
  return res;
}

int md_range(const std::string& policy_dir, const std::string& range_file_name, const std::string& file_name) {
  md_factory = init(policy_dir);

  metadata_memory_map_t map;

  if (!load_range_file(&map, range_file_name))
    return 1;

  if (!save_tags(&map, file_name)) {
    std::printf("failed write of tag file\n");
    return 1;
  }

  free(md_factory);
  return 0;
}

} // namespace policy_engine