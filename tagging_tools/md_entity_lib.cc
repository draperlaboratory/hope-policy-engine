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
#include <string>
#include <vector>
#include "basic_elf_io.h"
#include "elf_loader.h"
#include "elf_utils.h"
#include "entity_binding.h"
#include "metadata_tool.h"
#include "symbol_table.h"
#include "validator_exception.h"

namespace policy_engine {

static symbol_t* get_symbol(symbol_table_t const* symtab, reporter_t* err, std::string name, bool needs_size, bool optional) {
  symbol_t* sym = symtab->find_symbol(name);
  if (sym) {
    if (needs_size && sym->get_size() == 0) {
      if(optional) {
          err->warning("symbol %s has zero size.\n", name.c_str());
      }
      else {
          err->error("symbol %s has zero size.\n", name.c_str());
          sym = nullptr;
      }
    }
  } else {
    if (!optional)
      err->error("symbol %s not found\n", name.c_str());
  }
  return sym;
}

// debugging code
void dump_ents(metadata_tool_t& md_tool) {
  std::list<std::string> ents;
  md_tool.factory()->enumerate(ents);
  std::printf("ents:\n");
  for (auto s: ents) {
    std::printf("  %s\n", s.c_str());
  }
}

void verify_entity_bindings(metadata_tool_t& md_tool,
			    std::list<std::unique_ptr<entity_binding_t>>& bindings,
			    reporter_t& err) {
  std::list<std::string> ents;
  md_tool.factory()->enumerate(ents);
  for (auto s: ents) {
    auto it = std::find_if(bindings.begin(), bindings.end(),
			   [&](std::unique_ptr<entity_binding_t>& peb) {
			     entity_binding_t const* eb = dynamic_cast<entity_binding_t const*>(peb.get());
			     return eb->entity_name == s;
			   });
    if (it == bindings.end()) {
      err.warning("Entity %s has no binding\n", s.c_str());
    }
  }
}

int md_entity(const std::string& policy_dir, const std::string& elf_file_name, const std::string& tag_file_name, const std::vector<std::string>& yaml_files, stdio_reporter_t& err, bool update) {
  int yaml_count = yaml_files.size();
  std::string entity_yaml = policy_dir + "/policy_entities.yml";

  try {
    metadata_tool_t md_tool(policy_dir.c_str());
    elf_image_t img(elf_file_name, err);
    symbol_table_t symtab;
    populate_symbol_table(&symtab, &img);

    if (update) {
      if (!md_tool.load_tag_info(tag_file_name.c_str())) {
        err.error("couldn't load tags from %s\n", tag_file_name);
        return 2;
      }
    }

    std::list<std::unique_ptr<entity_binding_t>> bindings;
    std::printf("parsin %s\n", entity_yaml.c_str());
    load_entity_bindings(entity_yaml.c_str(), bindings, &err);
    for (const std::string& yaml_file : yaml_files) {
      std::printf("parsing %s\n", yaml_file.c_str());
      load_entity_bindings(yaml_file.c_str(), bindings, &err);
    }
    verify_entity_bindings(md_tool, bindings, err);

    for (auto &e: bindings) {
      entity_symbol_binding_t* sb = dynamic_cast<entity_symbol_binding_t*>(e.get());
      if (sb != nullptr) {
        symbol_t* sym = get_symbol(&symtab, &err, sb->elf_name, !sb->is_singularity, sb->optional);
        if (sym) {
          // go ahead and mark it
          uint64_t end_addr;
          if (sb->is_singularity)
            end_addr = sym->get_address() + (img.is_64bit() ? 8 : 4);
          else
            end_addr = sym->get_address() + sym->get_size(); // TODO: align to platform word boundary?
          if (!md_tool.apply_tag(sym->get_address(), end_addr, sb->entity_name.c_str())) {
            err.warning("Unable to apply tag %s\n", sb->entity_name.c_str());
          }
        }
      } else {
        entity_range_binding_t* rb = dynamic_cast<entity_range_binding_t*>(e.get());
        if (rb != nullptr) {
          symbol_t* sym = get_symbol(&symtab, &err, rb->elf_start_name, false, false);
          symbol_t* end = get_symbol(&symtab, &err, rb->elf_end_name, false, false);
          if (sym && end) {
            if (!md_tool.apply_tag(sym->get_address(), end->get_address(), rb->entity_name.c_str())) {
              err.warning("Unable to apply tag %s\n", rb->entity_name.c_str());
            }
          }
        }
      }
    }

    if (err.errors == 0) {
      if (!md_tool.save_tag_info(tag_file_name.c_str())) {
	      err.error("couldn't save tags to %s\n", tag_file_name);
      }
    }
  } catch (std::exception& e) {
    err.error("exception: %s\n", e.what());
  }

  return err.errors;
}

}