#include <cstdint>
#include <gelf.h>
#include <iostream>
#include <string>
#include "elf_loader.h"
#include "range_map.h"
#include "reporter.h"

namespace policy_engine {

static const std::string RWX_X = "elf.Section.SHF_EXECINSTR";
static const std::string RWX_R = "elf.Section.SHF_ALLOC";
static const std::string RWX_W = "elf.Section.SHF_WRITE";

void add_rwx_ranges(range_map_t& range_map, const elf_image_t& ef, reporter_t& err) {
  for (const elf_section_t& section : ef.sections) {
    if (section.flags & SHF_EXECINSTR) {
      range_map.add_range(section.address, section.end_address(), {RWX_R, RWX_X});
      err.info("X %s: %#lx - %#lx\n", section.name, section.address, section.end_address());
    } else if (section.flags & SHF_WRITE) {
      range_map.add_range(section.address, section.end_address(), RWX_W);
      err.info("W %s: %#lx - %#lx\n", section.name, section.address, section.end_address());
    } else if (section.flags & SHF_ALLOC) {
      range_map.add_range(section.address, section.end_address(), RWX_R);
      err.info("R %s: %#lx - %#lx\n", section.name, section.address, section.end_address());
    }
  }
}

}