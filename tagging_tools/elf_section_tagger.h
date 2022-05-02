#ifndef __ELF_SECTION_TAGGER_H__
#define __ELF_SECTION_TAGGER_H__

#include "elf_loader.h"
#include "reporter.h"
#include "tagging_utils.h"

namespace policy_engine {

void generate_rwx_ranges(const elf_image_t& ef, RangeFile& range_file, reporter_t& err);

}

#endif // __ELF_SECTION_TAGGER_H__