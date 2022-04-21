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

#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "asm_annotater.h"
#include "metadata_factory.h"
#include "tag_file.h"

#if defined(__clang__)
  // Clang pretends to be __GNUC__ 4.2.1; make_unique seems to be
  // supported on clang 3.4+.
  #define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
  #if CLANG_VERSION < 30400
    #include "make_unique.h"
  #endif
#elif defined(__GNUC__)
  #define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
  #if GCC_VERSION < 40900
    #include "make_unique.h"
  #endif
#endif

namespace policy_engine {

class annotater_t : public asm_annotater_t {
  metadata_factory_t& md_factory;
  metadata_memory_map_t& md_map;
  public:
  annotater_t(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, std::istream& in, std::ostream& out) :
    asm_annotater_t(in, out), md_factory(md_factory), md_map(md_map) { }
  std::string filter(uint64_t addr, std::string line);
};

std::string annotater_t::filter(uint64_t addr, std::string line) {
  metadata_t const* metadata = md_map.get_metadata(addr);
  if (!metadata)
    return line;
  
  line = asm_annotater_t::pad(line, 80);
  line += md_factory.render(metadata, true);
  return line;
}

int md_asm_ann(const std::string& policy_dir, const std::string& taginfo_file, const std::string& asm_file, const std::string& output_file) {
  metadata_memory_map_t md_map;

  std::unique_ptr<std::ifstream> asm_in;
  std::unique_ptr<std::ofstream> asm_out;
  std::unique_ptr<metadata_factory_t> md_factory;

  try {
    asm_in = std::make_unique<std::ifstream>(asm_file);
  } catch (...) {
    fprintf(stderr, "Couldn't open asm file %s\n", asm_file.c_str());
    return 1;
  }

  const std::string fname = output_file.empty() ? asm_file + ".tagged" : output_file;
  try {
    asm_out = std::make_unique<std::ofstream>(fname);
  } catch (...) {
    fprintf(stderr, "Couldn't open output asm file %s\n", fname.c_str());
    return 1;
  }
    
  try {
    md_factory = std::make_unique<metadata_factory_t>(policy_dir);
  } catch (...) {
    fprintf(stderr, "Couldn't load policy information from  %s\n", policy_dir.c_str());
    return 1;
  }

  if (!load_tags(&md_map, taginfo_file)) {
    fprintf(stderr, "Couldn't load tags from %s\n", taginfo_file.c_str());
    return 1;
  }
  annotater_t ann(*md_factory, md_map, *asm_in, *asm_out);
  ann.execute();
  return 0;
}

} // namespace policy_engine