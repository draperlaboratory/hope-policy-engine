#include <cstdio>
#include <gelf.h>
#include <stdexcept>
#include <string>
#include <vector>
#include "elf_loader.h"
#include "md_code.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void tag_op_codes(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const elf_image_t& ef, reporter_t& err) {
  for (const elf_section_t& section : ef.sections)
    if (section.flags & SHF_EXECINSTR)
      md_code(md_factory, md_map, section.address, section.data, section.size, err);
}

} // namespace policy_engine