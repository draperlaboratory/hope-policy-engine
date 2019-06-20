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

using namespace policy_engine;

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

void usage() {
  printf("usage: md_range <code_start> <code_end> <data_start> <data_end> <tag_file> <-m32/-m64> (default -m32)\n");
}

int main(int argc, char **argv) {
  const char *file_name;
  uintptr_t code_start, code_end;
  uintptr_t data_start, data_end;
  bool is_64_bit = false;

  if (argc < 6) {
    usage();
    return 0;
  }

  code_start = strtoul(argv[1], NULL, 10);
  code_end = strtoul(argv[2], NULL, 10);

  data_start = strtoul(argv[3], NULL, 10);
  data_end = strtoul(argv[4], NULL, 10);

  file_name = argv[5];

  if(argc == 7) {
    if(strcmp(argv[6], "-m64") == 0) {
      is_64_bit = true;
    }
  }

  if(write_headers(std::make_pair(code_start, code_end),
                   std::make_pair(data_start, data_end),
                   is_64_bit, std::string(file_name)) == false) {
    return 1;
  }

  return 0;
}
