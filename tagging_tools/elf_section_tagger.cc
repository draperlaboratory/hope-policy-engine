#include <cstdint>
#include <gelf.h>
#include <iostream>
#include <string>
#include "elf_loader.h"
#include "tagging_utils.h"

namespace policy_engine {

static const std::string RWX_X = "elf.Section.SHF_EXECINSTR";
static const std::string RWX_R = "elf.Section.SHF_ALLOC";
static const std::string RWX_W = "elf.Section.SHF_WRITE";

void generate_rwx_ranges(const elf_image_t& ef, RangeFile& range_file) {
  for (int i = 0; i < ef.get_shdr_count(); i++) {
    uint64_t flags = ef.get_shdrs()[i].sh_flags;
    uint64_t start = ef.get_shdrs()[i].sh_addr;
    uint64_t end = start + ef.get_shdrs()[i].sh_size;
    if (end % 4 != 0)
      end += 4 - (end % 4);
    if (flags & SHF_EXECINSTR) {
      range_file.write_range(start, end, RWX_X);
      range_file.write_range(start, end, RWX_R);
      std::printf("X %s: %#lx - %#lx\n", ef.get_section_name(i), start, end);
    } else if (flags & SHF_WRITE) {
      range_file.write_range(start, end, RWX_W);
      std::printf("W %s: %#lx - %#lx\n", ef.get_section_name(i), start, end);
    } else if (flags & SHF_ALLOC) {
      range_file.write_range(start, end, RWX_R);
      std::printf("R %s: %#lx - %#lx\n", ef.get_section_name(i), start, end);
    }
  }
}

}