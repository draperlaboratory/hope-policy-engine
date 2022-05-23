#ifndef __MD_EMBED_H__
#define __MD_EMBED_H__

#include <string>
#include "elf_loader.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void md_embed(metadata_factory_t& metadata_factory, metadata_memory_map_t& metadata_memory_map, elf_image_t& img, const std::string& elf_filename, reporter_t& err);
    
}

#endif // __MD_EMBED_H__