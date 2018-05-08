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

#include <stdio.h>
#include <gflags/gflags.h>

//#include <experimental/filesystem>

#include "basic_elf_io.h"
#include "elf_loader.h"
#include "symbol_table.h"
#include "entity_binding.h"
#include "validator_exception.h"
#include "elf_utils.h"
#include "metadata_tool.h"

using namespace policy_engine;

DEFINE_bool(update, true, "update existing tag info file");

static symbol_t *get_symbol(symbol_table_t const *symtab, reporter_t *err, std::string name, bool needs_size) {
  symbol_t *sym = symtab->find_symbol(name);
  if (sym) {
    if (needs_size && sym->get_size() == 0) {
      err->error("symbol %s has zero size.\n", name.c_str());
      sym = nullptr;
    }
  } else {
    err->error("symbol %s not found\n", name.c_str());
  }
  return sym;
}

static const char *usage_msg =
    "md_entity usage: md_entity <flags> policy_dir elf_file tag_info_file [entity files]\n"
    "  Applies metadata to the memory map of an ELF image according to bindings\n"
    "  described in entity YML files.  The file entities.yml in the given policy\n"
    "  directory will always be processed.  Additional optional entity files may be\n"
    "  provided on the command line.\n";
static void usage() {
  puts(usage_msg);
}

int main(int argc, char **argv) {
  stdio_reporter_t err;

  gflags::SetUsageMessage(usage_msg);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (argc < 4) {
    usage();
    return 0;
  }

  const char *policy_dir = argv[1];
  const char *elf_file_name = argv[2];
  const char *tag_file_name = argv[3];
  int yaml_count = argc - 4;
  char **yaml_files = &argv[4];
  std::string entity_yaml = std::string(policy_dir) + "/entities.yml";

  try {
    FILE *elf_in;

    metadata_tool_t md_tool(policy_dir);
    elf_in = fopen(elf_file_name, "rb");
    FILE_reader_t reader(elf_in);
    elf_image_t img(&reader, &err);
    symbol_table_t symtab;
    img.load();
    populate_symbol_table(&symtab, &img);

    if (FLAGS_update) {
      if (!md_tool.load_tag_info(tag_file_name)) {
	err.error("couldn't load tags from %s\n", tag_file_name);
	return 2;
      }
    }

    do {
      std::list<std::unique_ptr<entity_binding_t>> bindings;
      load_entity_bindings(entity_yaml.c_str(), bindings);
      for (auto &e: bindings) {
	entity_symbol_binding_t *sb = dynamic_cast<entity_symbol_binding_t *>(e.get());
	if (sb != nullptr) {
	  symbol_t *sym = get_symbol(&symtab, &err, sb->elf_name, !sb->is_singularity);
	  if (sym) {
	    // go ahead and mark it
	    address_t end_addr;
	    if (sb->is_singularity)
	      end_addr = sym->get_address() + PLATFORM_WORD_SIZE;
	    else
	      end_addr = sym->get_address() + sym->get_size(); // TODO: align to platform word boundary?
	    if (!md_tool.apply_tag(sym->get_address(), end_addr, sb->entity_name.c_str())) {
	      err.warning("Unable to apply tag %s\n", sb->entity_name.c_str());
	    }
	  }
	} else {
	  entity_range_binding_t *rb = dynamic_cast<entity_range_binding_t *>(e.get());
	  symbol_t *sym = get_symbol(&symtab, &err, rb->elf_start_name, false);
	  symbol_t *end = get_symbol(&symtab, &err, rb->elf_end_name, false);
	  if (sym && end) {
	    if (!md_tool.apply_tag(sym->get_address(), end->get_address(), rb->entity_name.c_str())) {
	      err.warning("Unable to apply tag %s\n", rb->entity_name.c_str());
	    }
	  }
	}
      }
      if (yaml_count)
	entity_yaml = *yaml_files++;
    } while (yaml_count--);

    if (err.errors == 0) {
      if (!md_tool.save_tag_info(tag_file_name)) {
	err.error("couldn't save tags to %s\n", tag_file_name);
      }
    }
  } catch (std::exception &e) {
    err.error("exception: %s\n", e.what());
  }

  return err.errors;
}
