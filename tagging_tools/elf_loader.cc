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

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <gelf.h>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "elf_loader.h"

namespace policy_engine {

elf_image_t::elf_image_t(const std::string& fname) : name(fname), fd(-1), elf(nullptr) {
  if (elf_version(EV_CURRENT) == EV_NONE)
    throw std::runtime_error(std::string("failed to initialize ELF library: ") + elf_errmsg(elf_errno()));
  fd = open(fname.c_str(), O_RDONLY);
  if (fd < 0)
    throw std::runtime_error("failed to open " + fname + ": " + std::strerror(errno));
  elf = elf_begin(fd, ELF_C_READ, nullptr);
  if (elf == nullptr)
    throw std::runtime_error(std::string("failed to initialize ELF file: ") + elf_errmsg(elf_errno()));
  else if (elf_kind(elf) != ELF_K_ELF)
    throw std::runtime_error(fname + " is not an ELF file");

  if (gelf_getehdr(elf, &ehdr) == nullptr)
    throw std::runtime_error(std::string("could not get ELF header: ") + elf_errmsg(elf_errno()));
  else if (!(ehdr.e_ident[0] == 0x7f && ehdr.e_ident[1] == 'E' && ehdr.e_ident[2] == 'L' && ehdr.e_ident[3] == 'F'))
    throw std::runtime_error("bad ELF signature");
  else if (ehdr.e_ident[4] == ELFCLASSNONE)
    throw std::runtime_error("could not determine ELF class");

  for (int i = 0; i < ehdr.e_shnum; i++) {
    Elf_Scn* scn = elf_getscn(elf, i);
    GElf_Shdr shdr;
    if (scn != nullptr && gelf_getshdr(scn, &shdr) != nullptr) {
      Elf_Data* data = nullptr;
      if ((data = elf_getdata(scn, data)) != nullptr) {
        sections.push_back({
          .name=elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name),
          .flags=shdr.sh_flags,
          .type=shdr.sh_type,
          .address=shdr.sh_addr,
          .offset=shdr.sh_offset,
          .size=shdr.sh_size,
          .data=data->d_buf
        });
      }
    }
  }

  for (int i = 0; i < ehdr.e_phnum; i++) {
    GElf_Phdr phdr;
    if (gelf_getphdr(elf, i, &phdr) != nullptr)
      program_headers.push_back(phdr);
  }

  auto strtab_scn = std::find_if(sections.begin(), sections.end(), [](const elf_section_t& s){ return s.name == ".strtab"; });
  char* strtab_bytes = reinterpret_cast<char*>(strtab_scn->data);
  for (int i = 1; i < strtab_scn->size; i++) {
    if (strtab_bytes[i - 1] == '\0')
      strtab.push_back(&strtab_bytes[i]);
  }

  int ind = std::find_if(sections.begin(), sections.end(), [](const elf_section_t& s){ return s.name == ".symtab"; }) - sections.begin();
  if (ind != sections.size()) {
    Elf_Scn* symtab_scn = elf_getscn(elf, ind);
    Elf_Data* symtab_data = nullptr;
    if ((symtab_data = elf_getdata(symtab_scn, symtab_data)) != nullptr) {
      int symbol_count = sections[ind].size/(word_bytes() == 8 ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym));
      for (int i = 0; i < symbol_count; i++) {
        GElf_Sym symbol;
        if (gelf_getsym(symtab_data, i, &symbol) != nullptr && symbol.st_shndx != SHN_UNDEF && symbol.st_shndx != SHN_ABS) {
          symtab.push_back(symbol_t{
            .name=strtab_bytes + symbol.st_name,
            .address=symbol.st_value & ~1,
            .size=symbol.st_size,
            .visibility=symbol_t::get_visibility(symbol),
            .kind=symbol_t::get_kind(symbol)
          });
        }
      }
    }
  }
}

elf_image_t::~elf_image_t() {
  if (elf != nullptr)
    elf_end(elf);
  if (fd >= 0)
    close(fd);
}

} // namespace policy_engine