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

#ifndef ENTITY_BINDING_H
#define ENTITY_BINDING_H

#include <list>
#include <memory>
#include <string>
#include "reporter.h"

namespace policy_engine {

struct entity_binding_t {
  const std::string entity_name;
  const bool optional;

  entity_binding_t(const std::string& n, bool o) : entity_name(n), optional(o) {}
  virtual ~entity_binding_t() {}
};

struct entity_symbol_binding_t : public entity_binding_t {
  /** ELF symbol the policy entity refers to. */
  const std::string elf_name;

  /** If true, we mark only the first word at the start symbol. If false, the symbol must have a size. */
  const bool is_singularity;

  entity_symbol_binding_t(const std::string& n, const std::string& elf, bool o=false, bool s=false) : entity_binding_t(n, o), elf_name(elf), is_singularity(s) {}
};

struct entity_range_binding_t : public entity_binding_t {
  const std::string elf_start_name;
  const std::string elf_end_name;

  entity_range_binding_t(const std::string& n, const std::string& start, const std::string& end, bool o=false) : entity_binding_t(n, o), elf_start_name(start), elf_end_name(end) {}
};

struct entity_soc_binding_t : public entity_binding_t {
  entity_soc_binding_t(const std::string& n, bool o=false) : entity_binding_t(n, o) {}
};

struct entity_isa_binding_t : public entity_binding_t {
  entity_isa_binding_t(const std::string& n, bool o=false) : entity_binding_t(n, o) {}
};

struct entity_image_binding_t : public entity_binding_t {
  entity_image_binding_t(const std::string& n, bool o=false) : entity_binding_t(n, o) {}
};

void load_entity_bindings(const std::string& file_name, std::list<std::unique_ptr<entity_binding_t>>& bindings, reporter_t& err);

} // namespace policy_engine

#endif
