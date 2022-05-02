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
#include "reporter.h"
#include "tag_file.h"

namespace policy_engine {

class annotater_t : public asm_annotater_t {
private:
  metadata_factory_t& md_factory;
  metadata_memory_map_t& md_map;

public:
  annotater_t(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, std::istream& in, std::ostream& out) :
    asm_annotater_t(in, out), md_factory(md_factory), md_map(md_map) {}

  std::string filter(uint64_t addr, std::string line) {
    const metadata_t* metadata = md_map.get_metadata(addr);
    if (!metadata)
      return line;
    return asm_annotater_t::pad(line, 80) + md_factory.render(metadata, true);
  }
};

int md_asm_ann(const std::string& policy_dir, const std::string& taginfo_file, const std::string& asm_file, reporter_t& err, const std::string& output_file) {
  metadata_memory_map_t md_map;

  std::ifstream asm_in(asm_file);
  if (!asm_in) {
    err.error("couldn't open asm file %s\n", asm_file);
    return 1;
  }

  const std::string fname = output_file.empty() ? asm_file + ".tagged" : output_file;
  std::ofstream asm_out(fname);
  if (!asm_out) {
    err.error("couldn't open output asm file %s\n", fname);
    return 1;
  }

  metadata_factory_t md_factory(policy_dir);

  if (!load_tags(&md_map, taginfo_file)) {
    err.error("couldn't load tags from %s\n", taginfo_file);
    return 1;
  }
  annotater_t ann(md_factory, md_map, asm_in, asm_out);
  ann.execute();
  return 0;
}

} // namespace policy_engine