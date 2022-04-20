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
#include <cstdio>
#include <string>
#include "tag_file.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "platform_types.h"
#include "riscv_isa.h"
#include "validator_exception.h"

namespace policy_engine {

int md_code(const std::string& policy_dir, address_t code_address, const std::string& file_name, uint8_t* bytes, int n) {
  try {
    metadata_factory_t* md_factory = init(policy_dir);
    metadata_memory_map_t map;
    if (!load_tags(&map, file_name)) {
      std::printf("failed read\n");
      std::fprintf(stderr, "failed to read tags from %s\n", file_name.c_str());
      return 1;
    }

    for (int i = 0; i < n/sizeof(insn_bits_t); i++) {
      insn_bits_t insn = reinterpret_cast<insn_bits_t*>(bytes)[i];
      uint32_t rs1, rs2, rs3, rd;
      int32_t imm;
      const char* name;
      uint32_t opdef;
      int32_t flags = decode(insn, &rs1, &rs2, &rs3, &rd, &imm, &name, &opdef);

      metadata_t const* metadata = md_factory->lookup_group_metadata(name, flags, rs1, rs2, rs3, rd, imm);

      if (metadata == nullptr) {
        std::fprintf(stderr, "0x%" PRIaddr_pad ": 0x%08x  %s - no group found for instruction\n", code_address, insn, name);
      } else {
        map.add_range(code_address, code_address + 4, metadata);
      }
      code_address += 4;
    }

    if (!save_tags(&map, file_name)) {
      std::printf("failed write of tag file\n");
      return 1;
    }
    
    free(md_factory);
    return 0;
  } catch (...) {
    std::printf("something awful happened\n");
    return 1;
  }
}

} // namespace policy_engine