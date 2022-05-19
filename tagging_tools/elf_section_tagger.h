#ifndef __ELF_SECTION_TAGGER_H__
#define __ELF_SECTION_TAGGER_H__

#include "elf_loader.h"
#include "reporter.h"
#include "tagging_utils.h"

namespace policy_engine {

void add_rwx_ranges(range_map_t& range_map, const elf_image_t& ef, reporter_t& err);

}

#endif // __ELF_SECTION_TAGGER_H__