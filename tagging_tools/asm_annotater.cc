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

#include <iostream>

#include "asm_annotater.h"

using namespace policy_engine;

void asm_annotater_t::execute() {
  std::string line;
  while (std::getline(input, line)) {
    bool stop = false;
    for (size_t i = 0; i < line.size() && !stop; i++) {
      if (!isspace(line[i])) {
	if (!isxdigit(line[i])) {
	  stop = true;
         if (i > 0 && line[i] == ':' && !isspace(line[i-1])) {
	    address_t addr;
	    addr = std::stoul(line, nullptr, 16);
	    output << filter(addr, line) << std::endl;
	  } else {
	    output << filter(line) << std::endl;
	  }
	}
      }
    }
    // edge case - entire line was nothing but numbers (can this happen?), or empty (just a newline)
    if (!stop) {
      output << filter(line) << std::endl;
    }
  }
}

std::string asm_annotater_t::pad(std::string str, int width) {
  std::string res;
  int len = 0;
  for (char c: str) {
    if (c == '\t') {
//      res += "        ";
//      len += 8;
      res += "    ";
      len += 4;
    } else {
      res += c;
      len ++;
    }
  }
  while (len < width) {
      res += " ";
      len++;
  }
  return res;
}

#if 0
class my_a : public asm_annotater_t {
  public:
  my_a(std::istream &in, std::ostream &out) : asm_annotater_t(in, out) { }
  virtual std::string filter(address_t addr, std::string line) { return "ADDR " + line; }
  virtual std::string filter(std::string line) { return "FOO " + line; }
};

int main() {
  my_a an(std::cin, std::cout);
  an.execute();
}
#endif
