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

#include <gelf.h>
#include "elf_utils.h"

namespace policy_engine {

void populate_symbol_table(symbol_table_t *symtab, elf_image_t *img) {
  GElf_Sym const *syms = img->get_symbols();
  for (int i = 0; i < img->get_symbol_count(); i++) {
    GElf_Sym const *esym = &syms[i];
    if (esym->st_shndx != SHN_UNDEF && esym->st_shndx != SHN_ABS) {
      symbol_t::visibility_t visibility;
      symbol_t::kind_t kind;
      switch (GELF_ST_BIND(esym->st_info)) {
        case STB_LOCAL: visibility = symbol_t::PRIVATE; break;
        case STB_GLOBAL: visibility = symbol_t::PUBLIC; break;
        default: visibility = symbol_t::PRIVATE; break;
      }
      switch (GELF_ST_TYPE(esym->st_info)) {
        case STT_FUNC: kind = symbol_t::CODE; break;
        default: kind = symbol_t::DATA; break;
      }
      symbol_t *sym = new symbol_t(img->get_string(esym->st_name), esym->st_value & ~1, esym->st_size, visibility, kind);
      symtab->add_symbol(sym);
    }
  }
}

void get_elf_sections(elf_image_t *img,
     std::list<GElf_Shdr const *>&code_sections,
     std::list<GElf_Shdr const *>&data_sections) {
  GElf_Shdr const *shdrs = img->get_shdrs();
  size_t i;
  unsigned int flags;

  for(i = 0; i < img->get_shdr_count(); i++) {
     if (shdrs[i].sh_flags & SHF_ALLOC) {
      flags = shdrs[i].sh_flags;
      if ((flags & (SHF_WRITE | SHF_EXECINSTR)) == (SHF_EXECINSTR)) {
        code_sections.push_back(&shdrs[i]);
      } else {
        data_sections.push_back(&shdrs[i]);
      }
    }
  }
}

} // namespace policy_engine