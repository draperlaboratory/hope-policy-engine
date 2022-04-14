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

#include <cstdlib>
#include <gelf.h>
#include <stdint.h>
#include <sys/types.h>
#include "reporter.h"

namespace policy_engine {

struct file_stream_t {
  enum whence_t { CUR, SET };
  virtual bool read(void *buff, size_t size) = 0;
  virtual bool seek(size_t where, whence_t whence) = 0;
};

class elf_image_t {
public:
  elf_image_t(file_stream_t *file, reporter_t *err);
  ~elf_image_t();
  bool load();
  bool is_64bit();
  uintptr_t get_entry_point() const;
  GElf_Ehdr get_ehdr() const { return eh; }
  GElf_Phdr const* get_phdrs() const { return phdrs; }
  int get_phdr_count() const { return eh.e_phnum; }
  GElf_Shdr const* get_shdrs() const { return shdrs; }
  int get_shdr_count() const { return eh.e_shnum; }
  const char *get_section_name(int sect_num) const;
  GElf_Shdr const *find_section(const char *name) const;

  bool load_bits(GElf_Shdr const *shdr, void **bits, const char *purpose) {
    return load_bits(bits, shdr->sh_size, shdr->sh_offset, purpose);
  }
  bool load_bits(void **bits, size_t size, off_t off, const char *description);

  const char *get_string(int str) const { if (str_tab) return str_tab + str; return 0; }
  bool find_symbol_addr(const char *name, uintptr_t &addr, size_t &size) const;
  GElf_Sym const *get_symbols() const { return sym_tab; }
  int get_symbol_count() const { return symbol_count; }

private:
  bool check_header_signature();
  GElf_Ehdr eh;
  GElf_Phdr *phdrs;
  GElf_Shdr *shdrs;
  char *sh_str_tab;
  char *str_tab;
  GElf_Sym *sym_tab;
  int symbol_count;
  file_stream_t *file;
  reporter_t *err;
};

} // namespace policy_engine

#endif // ELF_LOADER_H
