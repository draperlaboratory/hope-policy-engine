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

#include <any>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include "tag_file.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "platform_types.h"
#include "reporter.h"
#include "riscv_isa.h"
#include "validator_exception.h"

namespace policy_engine {

void md_code(const std::string& policy_dir, uint64_t code_address, const std::string& file_name, void* bytes, int n, reporter_t& err) {
  metadata_factory_t md_factory(policy_dir);
  metadata_memory_map_t map;
  if (!load_tags(map, file_name))
    throw std::ios::failure("failure to read tags from " + file_name);

  insn_bits_t* bits = reinterpret_cast<insn_bits_t*>(bytes);
  for (int i = 0; i < n/sizeof(insn_bits_t); i++) {
    decoded_instruction_t inst = decode(bits[i]);
    if (!inst) {
      err.warning("Failed to decode instruction 0x%08x at address %#x\n", bits[i], code_address);
      code_address += 4;
      continue;
    }

    std::shared_ptr<metadata_t> metadata = md_factory.lookup_group_metadata(inst.name, inst);

    if (metadata == nullptr) {
      err.warning("0x%016lx: 0x%08x  %s - no group found for instruction\n", code_address, bits[i], inst.name);
    } else {
      map.add_range(code_address, code_address + 4, metadata);
    }
    code_address += 4;
  }

  if (!save_tags(map, file_name))
    throw std::ios::failure("failed write of tag file");
}

} // namespace policy_engine