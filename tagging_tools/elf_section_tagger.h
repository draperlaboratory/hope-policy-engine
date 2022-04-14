#ifndef __ELF_SECTION_TAGGER_H__
#define __ELF_SECTION_TAGGER_H__

#include "elf_loader.h"
#include "tagging_utils.h"

namespace policy_engine {

void generate_rwx_ranges(const elf_image_t& ef, RangeFile& range_file);

}

#endif // __ELF_SECTION_TAGGER_H__