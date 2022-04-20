#include <cstdio>
#include <gelf.h>
#include <string>
#include <vector>
#include "elf_loader.h"

namespace policy_engine {

void tag_op_codes(const std::string& policy_dir, const std::string& md_code, elf_image_t& ef, const std::string& taginfo_file_name) {
  for (int i = 0; i < ef.get_shdr_count(); i++) {
    if (ef.get_shdrs()[i].sh_flags & SHF_EXECINSTR) {
      char buf[11];
      sprintf(buf, "0x%08lx", ef.get_shdrs()[i].sh_addr);
      std::string code_cmd = md_code + " " + policy_dir + " " + buf + " " + taginfo_file_name;
      std::printf("%s\n", code_cmd.c_str());
      std::FILE* proc = popen(code_cmd.c_str(), "w");
      uint8_t* data;
      ef.load_bits(&ef.get_shdrs()[i], (void**)&data, "section data");
      std::fwrite(data, ef.get_shdrs()[i].sh_size, 1, proc);
      pclose(proc);
      free(data);
    }
  }
}

} // namespace policy_engine