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
#include "metadata_factory.h"
#include "validator_exception.h"

#include "riscv_isa.h"

using namespace policy_engine;

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

void usage() {
  printf("usage: md_code <policy-dir> <code_address> <tag_file>\n");
  printf("  Reads a stream of binary instructions from stdin, applying metadata\n");
  printf("  for each instruction related to the group of operations that instruction\n");
  printf("  may represent.\n");
}

struct abstract_instruction_stream_t {
  virtual ~abstract_instruction_stream_t() { }
  virtual bool read_instruction(insn_bits_t &insn) = 0;
  virtual void read_error() = 0;
};

struct read_error_t { };

class rv32_insn_stream_t : public abstract_instruction_stream_t {
  FILE *fp;
  public:
  rv32_insn_stream_t(FILE *fp) : fp(fp) { }
  virtual ~rv32_insn_stream_t() { }
  virtual bool read_instruction(insn_bits_t &insn) {
    if (fread(&insn, sizeof(insn_bits_t), 1, fp) != 1) {
      if (!feof(fp))
	read_error();
      return false;
    }
    return true;
  }
  virtual void read_error() { throw read_error_t(); }
};

int main(int argc, char **argv) {
try {
  const char *policy_dir;
  address_t code_address;
  const char *file_name;

  if (argc != 4) {
    usage();
    return 0;
  }

  policy_dir = argv[1];
  code_address = strtoul(argv[2], 0, 16);
  file_name = argv[3];

  init(policy_dir);
  metadata_memory_map_t map;
  if (!load_tags(&map, file_name)) {
    printf("failed read\n");
    fprintf(stderr, "failed to read tags from %s\n", file_name);
    return 1;
  }

// use this for debugging with gdb
//  FILE *foo = fopen("/tmp/bits.bin", "rb");
//  rv32_insn_stream_t s(foo);
  rv32_insn_stream_t s(stdin);
  insn_bits_t insn;
  try {
    int cnt = 0;
    while (s.read_instruction(insn)) {
      uint32_t rs1, rs2, rs3, rd;
      int32_t imm;
      const char *name;
      uint32_t opdef;
      int32_t flags = decode(insn, &rs1, &rs2, &rs3, &rd, &imm, &name, &opdef);

      metadata_t const *metadata =
        md_factory->lookup_group_metadata(name, flags, rs1, rs2, rs3, rd, imm);

      if (metadata == nullptr) {
         fprintf(stderr, "0x%" PRIaddr_pad
                 ": 0x%08x  %s - no group found for instruction\n",
                 code_address, insn, name);
      } else {
        // std::string s = md_factory->render(metadata);
        // printf("0x%08x: %s\n", code_address, s.c_str());

        map.add_range(code_address, code_address + 4, metadata);
      }
      code_address += 4;
    }
  } catch (read_error_t &e) {
    fprintf(stderr, "read error on stdin\n");
    return 1;
  }
  
//  printf("writing tags to %s\n", file_name);
  if (!save_tags(&map, file_name)) {
    printf("failed write of tag file\n");
    return 1;
  }
  
  return 0;
} catch (...) {
  printf("something awful happened\n");
  return 1;
}
}
