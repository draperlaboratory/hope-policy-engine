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

#include <algorithm>
#include <cstdio>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>
#include "elf_loader.h"
#include "entity_binding.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"
#include "symbol_table.h"
#include "validator_exception.h"

namespace policy_engine {

// debugging code
void dump_ents(metadata_factory_t& md_factory) {
  std::printf("ents:\n");
  for (const std::string& s: md_factory.enumerate()) {
    std::printf("  %s\n", s.c_str());
  }
}

void md_entity(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const elf_image_t& img, const std::vector<std::string>& yaml_files, reporter_t& err) {
  std::list<std::unique_ptr<entity_binding_t>> bindings;
  for (const std::string& yaml_file : yaml_files)
    load_entity_bindings(yaml_file, bindings, err);
  for (const std::string& s : md_factory.enumerate()) {
    auto it = std::find_if(bindings.begin(), bindings.end(), [&](std::unique_ptr<entity_binding_t>& peb) { return peb->entity_name == s; });
    if (it == bindings.end()) {
      err.warning("Entity %s has no binding\n", s);
    }
  }

  for (const std::unique_ptr<entity_binding_t>& e: bindings) {
    if (const auto sb = dynamic_cast<entity_symbol_binding_t*>(e.get())) {
      if (auto sym = img.symtab.find(sb->elf_name, !sb->is_singularity, sb->optional, err); sym != img.symtab.end()) {
        // go ahead and mark it
        uint64_t end_addr;
        if (sb->is_singularity)
          end_addr = sym->address + img.word_bytes();
        else
          end_addr = sym->address + sym->size; // TODO: align to platform word boundary?
        if (!md_factory.apply_tag(md_map, sym->address, end_addr, sb->entity_name)) {
          err.warning("Unable to apply tag %s\n", sb->entity_name);
        }
      }
    } else if (const auto rb = dynamic_cast<entity_range_binding_t*>(e.get())) {
      auto sym = img.symtab.find(rb->elf_start_name, false, false, err);
      auto end = img.symtab.find(rb->elf_end_name, false, false, err);
      if (sym != img.symtab.end() && end != img.symtab.end()) {
        if (!md_factory.apply_tag(md_map, sym->address, end->address, rb->entity_name)) {
          err.warning("Unable to apply tag %s\n", rb->entity_name);
        }
      }
    }
  }
}

}