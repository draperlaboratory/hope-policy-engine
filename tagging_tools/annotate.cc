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
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include "metadata.h"
#include "metadata_memory_map.h"
#include "metadata_factory.h"
#include "reporter.h"
#include "tag_file.h"

namespace policy_engine {

std::string pad(const std::string& str, int width) {
  std::string res;
  for (char c: str) {
    if (c == '\t') {
      res += "    ";
    } else {
      res += c;
    }
  }
  while (res.size() < width) {
    res += " ";
  }
  return res;
}

void annotate_asm(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const std::string& asm_file, const std::string& output_file) {
  std::ifstream asm_in(asm_file);
  if (!asm_in)
    throw std::ios::failure("couldn't open input file " + asm_file);
  const std::string fname = output_file.empty() ? asm_file + ".tagged" : output_file;
  std::ofstream asm_out(fname);
  if (!asm_out)
    throw std::ios::failure("couldn't open output file " + fname);

  for (std::string line; std::getline(asm_in, line);) {
    bool stop = false;
    for (std::size_t i = 0; i < line.size() && !stop; i++) {
      if (!isspace(line[i]) && !isxdigit(line[i])) {
        stop = true;
        if (i > 0 && line[i] == ':' && !isspace(line[i - 1])) {
          if (const metadata_t* metadata = md_map.get_metadata(std::stoul(line.substr(0, i - 1), nullptr, 16)))
            asm_out << pad(line, 80) + md_factory.render(metadata, true) << std::endl;
          else
            asm_out << line << std::endl;
        } else {
          asm_out << line << std::endl;
        }
      }
    }
    // edge case - entire line was nothing but numbers (can this happen?), or empty (just a newline)
    if (!stop) {
      asm_out << line << std::endl;
    }
  }
}

} // namespace policy_engine