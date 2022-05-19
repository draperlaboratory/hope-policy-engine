#include <cstdio>
#include <gelf.h>
#include <stdexcept>
#include <string>
#include <vector>
#include "elf_loader.h"
#include "md_code.h"
#include "reporter.h"

namespace policy_engine {

void tag_op_codes(const std::string& policy_dir, elf_image_t& ef, const std::string& taginfo_file_name, reporter_t& err) {
  for (const elf_section_t& section : ef.sections)
    if (section.flags & SHF_EXECINSTR)
      md_code(policy_dir, section.address, taginfo_file_name, section.data, section.size, err);
}

} // namespace policy_engine