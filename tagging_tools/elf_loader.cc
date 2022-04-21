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
#include <gelf.h>
#include <string>
#include "elf_loader.h"
#include "basic_elf_io.h" // not alphabetical, but FILE_reader_t needs to be declared after file_stream_t

namespace policy_engine {

elf_image_t::elf_image_t(const std::string& fname, reporter_t& err) : phdrs(nullptr), shdrs(nullptr), str_tab(nullptr), sym_tab(nullptr), err(err) {
  file = new FILE_reader_t(fopen(fname.c_str(), "rb"));
  valid = file != nullptr && load();
}

elf_image_t::~elf_image_t() {
  if (file != nullptr) {
    fclose(static_cast<FILE_reader_t*>(file)->fp);
    free(file);
  }
  if (phdrs != nullptr)
    free(phdrs);
  if (shdrs != nullptr)
    free(shdrs);
  if (str_tab != nullptr)
    free(str_tab);
  if (sym_tab != nullptr)
    free(sym_tab);
}

bool elf_image_t::check_header_signature() {
  return eh.e_ident[0] == 0x7f && eh.e_ident[1] == 'E' && eh.e_ident[2] == 'L' && eh.e_ident[3] == 'F';
}

bool elf_image_t::is_64bit() {
  return eh.e_ident[4] == ELFCLASS64;
}

bool elf_image_t::load_bits(void **bits, size_t size, off_t off, const char *description) {
  *bits = malloc(size);
  if (!*bits) {
    err.error("unable to allocate %s\n", description);
    return false;
  }
  if (!file->seek(off, file_stream_t::SET) || !file->read(*bits, size)) {
    err.error("file I/O error reading %s\n", description);
    free(*bits);
    *bits = 0;
    return false;
  }
  return true;
}

const char *elf_image_t::get_section_name(int sect_num) const {
  if (sh_str_tab) {
    GElf_Shdr const *sh = &get_shdrs()[sect_num];
    return sh_str_tab + sh->sh_name;
  }
  return NULL;
}

bool elf_image_t::find_symbol_addr(const std::string& name, uintptr_t &addr, size_t &size) const {
  if (str_tab && sym_tab) {
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

bool elf_image_t::load() {
  if (!file->read(&eh, sizeof(eh))) {
    err.error("error reading header");
    return false;
  }
  if (!check_header_signature()) {
    err.error("bad ELF signature");
    return false;
  }
  
  if ((eh.e_phentsize != sizeof(*phdrs)) || (eh.e_shentsize != sizeof(*shdrs))) {
    err.error("elf header inconsistency");
    return false;
  }
  
  if (!load_bits((void **)&phdrs, eh.e_phnum * eh.e_phentsize, eh.e_phoff, "program headers"))
    return false;
  if (!load_bits((void **)&shdrs, eh.e_shnum * eh.e_shentsize, eh.e_shoff, "section headers"))
    return false;

  // Should validate the header's section index here...
  GElf_Shdr const *str_sh = &get_shdrs()[eh.e_shstrndx];
  if (!load_bits((void **)&sh_str_tab, str_sh->sh_size, str_sh->sh_offset, "section string table"))
    return false;

  str_sh = find_section(".strtab");
  if (!str_sh) {
    err.error("cound't find strtab\n");
    return false;
  }
  if (!load_bits((void **)&str_tab, str_sh->sh_size, str_sh->sh_offset, "string table"))
    return false;
  GElf_Shdr const *sym_sh = find_section(".symtab");
  if (!sym_sh)
    return false;
  if (!load_bits((void **)&sym_tab, sym_sh->sh_size, sym_sh->sh_offset, "symbol table"))
    return false;
  symbol_count = sym_sh->sh_size / sizeof(GElf_Sym);
  if (sym_sh->sh_size % sizeof(GElf_Sym)) {
    err.error("section size not mod sizeof elf_sym\n");
    return false;
  }

  return true;
}

uintptr_t elf_image_t::get_entry_point() const {
  return eh.e_entry;
}

GElf_Shdr const* elf_image_t::find_section(const std::string& name) const {
  // We have to have the string table loaded for this.
  if (sh_str_tab) {
    for (int i = 0; i < get_shdr_count(); i++) {
      if (name == get_section_name(i))
	      return &get_shdrs()[i];
    }
  }
  return NULL;
}

} // namespace policy_engine