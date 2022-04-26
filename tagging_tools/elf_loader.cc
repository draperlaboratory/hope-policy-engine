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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <gelf.h>
#include <string>
#include <unistd.h>
#include "elf_loader.h"
#include "basic_elf_io.h" // not alphabetical, but FILE_reader_t needs to be declared after file_stream_t

namespace policy_engine {

elf_image_t::elf_image_t(const std::string& fname, reporter_t& err) : fd(-1), elf(nullptr), sym_tab(nullptr), err(err) {
  valid = true;
  if (elf_version(EV_CURRENT) == EV_NONE) {
    valid = false;
    err.error("failed to initialize ELF library: %s\n", elf_errmsg(elf_errno()));
  }
  if (valid) {
    fd = open(fname.c_str(), O_RDONLY);
    if (fd < 0) {
      valid = false;
      err.error("failed to open %s: %s\n", fname.c_str(), std::strerror(errno));
    }
  }
  if (valid) {
    elf = elf_begin(fd, ELF_C_READ, nullptr);
    if (elf == nullptr) {
      valid = false;
      err.error("failed to initialize ELF file: %s\n", elf_errmsg(elf_errno()));
    } else if (elf_kind(elf) != ELF_K_ELF) {
      valid = false;
      err.error("%s is not an ELF file\n", fname.c_str());
    }
  }
  if (valid) {
    if (gelf_getehdr(elf, &eh) == nullptr) {
      valid = false;
      err.error("could not get ELF header: %s\n", elf_errmsg(elf_errno()));
    } else if (!(eh.e_ident[0] == 0x7f && eh.e_ident[1] == 'E' && eh.e_ident[2] == 'L' && eh.e_ident[3] == 'F')) {
      valid = false;
      err.error("bad ELF signature\n");
    } else if (eh.e_ident[4] == ELFCLASSNONE) {
      valid = false;
      err.error("could not determine ELF class\n");
    }
  }
  if (valid) {
    for (int i = 0; i < eh.e_shnum; i++) {
      Elf_Scn* scn = elf_getscn(elf, i);
      GElf_Shdr shdr;
      if (scn == nullptr) {
        err.error("could not get section %d: %s", i, elf_errmsg(elf_errno()));
      } else if (gelf_getshdr(scn, &shdr) == nullptr) {
        err.error("could not get section header %d: %s\n", i, elf_errmsg(elf_errno()));
      } else {
        Elf_Data* data = nullptr;
        if ((data = elf_getdata(scn, data)) == nullptr)
          err.error("could not load section %d data: %s\n", i, elf_errmsg(elf_errno()));
        sections.push_back({
          .name=elf_strptr(elf, eh.e_shstrndx, shdr.sh_name),
          .flags=shdr.sh_flags,
          .type=shdr.sh_type,
          .address=shdr.sh_addr,
          .offset=shdr.sh_offset,
          .size=shdr.sh_size,
          .data=data->d_buf
        });
      }
    }

    for (int i = 0; i < eh.e_phnum; i++) {
      GElf_Phdr phdr;
      if (gelf_getphdr(elf, i, &phdr) == nullptr)
        err.error("could not get program header %d: %s\n", i, elf_errmsg(elf_errno()));
      else
        program_headers.push_back(phdr);
    }

    str_tab = reinterpret_cast<char*>(find_section(".strtab")->data);

    int sym;
    for (sym = 0; sym < sections.size(); sym++) {
      if (sections[sym].name == ".symtab")
        break;
    }
    if (sym == eh.e_shnum)
      err.error("could not find section .symtab\n");
    else {
      Elf_Scn* symtab_scn = elf_getscn(elf, sym);
      Elf_Data* symtab_data = nullptr;
      if ((symtab_data = elf_getdata(symtab_scn, symtab_data)) == nullptr)
        err.error("could not load .symtab: %s\n", elf_errmsg(elf_errno()));
      else {
        symbol_count = sections[sym].size/(is_64bit() ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym));
        sym_tab = new GElf_Sym[symbol_count];
        for (int i = 0; i < symbol_count; i++) {
          if (gelf_getsym(symtab_data, i, &sym_tab[i]) == nullptr)
            err.error("could not load .symtab symbol %d: %s\n", i, elf_errmsg(elf_errno()));
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
  if (sym_tab != nullptr)
    free(sym_tab);
}

bool elf_image_t::find_symbol_addr(const std::string& name, uintptr_t &addr, size_t &size) const {
  if (sym_tab) {
    GElf_Sym const *syms = sym_tab;
    for (int i = 0; i < symbol_count; syms++, i++) {
      if (name == get_string(syms->st_name)) {
        addr = syms->st_value;
        size = syms->st_size;
        return true;
      }
    }
  }
  return false;
}

const elf_section_t* elf_image_t::find_section(const std::string& name) const {
  for (const auto& section : sections) {
    if (section.name == name)
      return &section;
  }
  err.error("could not find section named %s", name.c_str());
  return nullptr;
}

} // namespace policy_engine