#include <cstdio>
#include <gelf.h>
#include <string>
#include <vector>
#include "elf_loader.h"
#include "md_code.h"

namespace policy_engine {

void tag_op_codes(const std::string& policy_dir, elf_image_t& ef, const std::string& taginfo_file_name) {
  for (int i = 0; i < ef.get_shdr_count(); i++) {
    if (ef.get_shdrs()[i].sh_flags & SHF_EXECINSTR) {
      uint8_t* data;
      ef.load_bits(&ef.get_shdrs()[i], (void**)&data, "section data");
      md_code(policy_dir, ef.get_shdrs()[i].sh_addr, taginfo_file_name, data, ef.get_shdrs()[i].sh_size);
      free(data);
    }
  }
}

} // namespace policy_engine