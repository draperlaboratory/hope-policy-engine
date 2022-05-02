#include <cstdint>
#include <gelf.h>
#include <iostream>
#include <string>
#include "elf_loader.h"
#include "reporter.h"
#include "tagging_utils.h"

namespace policy_engine {

static const std::string RWX_X = "elf.Section.SHF_EXECINSTR";
static const std::string RWX_R = "elf.Section.SHF_ALLOC";
static const std::string RWX_W = "elf.Section.SHF_WRITE";

void generate_rwx_ranges(const elf_image_t& ef, RangeFile& range_file, reporter_t& err) {
  for (const auto& section : ef.sections) {
    uint64_t end = section.address + section.size;
    if (end % 4 != 0)
      end += 4 - (end % 4);
    if (section.flags & SHF_EXECINSTR) {
      range_file.write_range(section.address, end, RWX_X);
      range_file.write_range(section.address, end, RWX_R);
      err.info("X %s: %#lx - %#lx\n", section.name.c_str(), section.address, end);
    } else if (section.flags & SHF_WRITE) {
      range_file.write_range(section.address, end, RWX_W);
      err.info("W %s: %#lx - %#lx\n", section.name.c_str(), section.address, end);
    } else if (section.flags & SHF_ALLOC) {
      range_file.write_range(section.address, end, RWX_R);
      err.info("R %s: %#lx - %#lx\n", section.name.c_str(), section.address, end);
    }
  }
}

}