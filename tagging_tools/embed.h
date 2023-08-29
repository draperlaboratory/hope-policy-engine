#ifndef __EMBED_H__
#define __EMBED_H__

#include <string>
#include "elf_loader.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void embed_tags(const metadata_memory_map_t& metadata_memory_map, const elf_image_t& img, const std::string& elf_filename, reporter_t& err);
    
}

#endif // __EMBED_H__