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
#include <fstream>
#include <memory>
#include <stdint.h>

#include <gflags/gflags.h>

#include "metadata_factory.h"
#include "tag_file.h"
#include "asm_annotater.h"

using namespace policy_engine;

class annotater_t : public asm_annotater_t {
  metadata_factory_t &md_factory;
  metadata_memory_map_t &md_map;
  public:
  annotater_t(metadata_factory_t &md_factory, metadata_memory_map_t &md_map,
	      std::istream &in, std::ostream &out) :
    asm_annotater_t(in, out), md_factory(md_factory), md_map(md_map) { }
  std::string filter(address_t addr, std::string line);
};

std::string annotater_t::filter(address_t addr, std::string line) {
  metadata_t const *metadata = md_map.get_metadata(addr);
  if (!metadata)
    return line;
  
  line = asm_annotater_t::pad(line, 80);
  line += md_factory.render(metadata, true);
  return line;
#if 0
  tag_space_t *space = tag_space_map.find(addr);
  if (!space) {
    printf("warning: no tag space found for 0x%x\n", addr);
    return line;
  }
  tag_set_t *ts = space->get_tags(addr);
  if (!ts)
    return line;
  line = asm_annotater_t::pad(line, 80);
  for (auto e: *ts) {
    switch (e) {
      case CFI_JUMP: line += "CFI_JUMP "; break;
      case CFI_TARGET: line += "CFI_TARGET "; break;
      case STACK_PROLOG: line += "PROLOG "; break;
      case STACK_EPILOG: line += "EPILOG "; break;
    default: line += "<what?>"; break;
    }
  }
#endif
  return line;
}

static void usage() {
  printf("usage: md_asm_ann policy_dir taginfo_file asm_file\n");
}

DEFINE_int32(base_address, 0x80000000, "Base address for memory map");
DEFINE_string(output, "", "Output asm file");

int main(int argc, char **argv) {
  gflags::SetUsageMessage("ann usage:\n");
  gflags::ParseCommandLineFlags(&argc, &argv, false);
//  printf("base addr = 0x%08x\n", FLAGS_base_address);
//  printf("output file = %s\n", FLAGS_output.c_str());
#if 1
  if (argc != 4) {
    usage();
    return 0;
  }
  const char *policy_dir = argv[1];
  const char *taginfo_file = argv[2];
  const char *asm_file = argv[3];

  if (FLAGS_output.size() == 0) {
    FLAGS_output = std::string(asm_file) + ".tagged";
  }
  
  metadata_cache_t md_cache;
  metadata_memory_map_t md_map(FLAGS_base_address, &md_cache);

  std::unique_ptr<std::ifstream> asm_in;
  std::unique_ptr<std::ofstream> asm_out;
  std::unique_ptr<metadata_factory_t> md_factory;

  try {
    asm_in = std::make_unique<std::ifstream>(asm_file);
  } catch (...) {
    fprintf(stderr, "Couldn't open asm file %s\n", asm_file);
    return 1;
  }

  try {
    asm_out = std::make_unique<std::ofstream>(FLAGS_output);
  } catch (...) {
    fprintf(stderr, "Couldn't open output asm file %s\n", FLAGS_output.c_str());
    return 1;
  }
    
  try {
    md_factory = std::make_unique<metadata_factory_t>(policy_dir);
  } catch (...) {
    fprintf(stderr, "Couldn't load policy information from  %s\n", policy_dir);
    return 1;
  }

  if (!load_tags(&md_map, taginfo_file)) {
    fprintf(stderr, "Couldn't load tags from %s\n", taginfo_file);
    return 1;
  }
  annotater_t ann(*md_factory, md_map, *asm_in, *asm_out);
  ann.execute();
  return 0;
#endif  
}
