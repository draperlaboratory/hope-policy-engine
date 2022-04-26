#include <cstdio>
#include <gelf.h>
#include <string>
#include <vector>
#include "elf_loader.h"
#include "md_code.h"

namespace policy_engine {

void tag_op_codes(const std::string& policy_dir, elf_image_t& ef, const std::string& taginfo_file_name) {
  for (const auto& section : ef.sections) {
    md_code(policy_dir, section.address, taginfo_file_name, reinterpret_cast<uint8_t*>(section.data), section.size);
  }
}

} // namespace policy_engine