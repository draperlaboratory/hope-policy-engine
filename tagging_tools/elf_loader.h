/*
 * Copyright © 2017-2018 The Charles Stark Draper Laboratory, Inc. and/or Dover Microsystems, Inc.
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

#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <cstdint>
#include <gelf.h>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <vector>
#include "symbol_table.h"
#include "tagging_utils.h"

namespace policy_engine {

struct elf_section_t {
  const std::string name;
  const uint64_t flags;
  const uint32_t type;
  const uint64_t address;
  const uint64_t offset;
  const size_t size;
  void* const data;

  constexpr uint64_t end_address() const { return round_up(address + size, 4); }
};

class elf_image_t {
private:
  int fd;
  Elf* elf;

public:
  const std::string name;
  GElf_Ehdr ehdr;
  std::vector<elf_section_t> sections;
  std::vector<GElf_Phdr> program_headers;
  symbol_table_t symtab;
  std::vector<std::string> strtab;

  elf_image_t(const std::string& fname);
  ~elf_image_t();

  int word_bytes() const { return ehdr.e_ident[4] == ELFCLASS64 ? 8 : 4; }
  uintptr_t entry_point() const { return ehdr.e_entry; }
};

} // namespace policy_engine

#endif // ELF_LOADER_H