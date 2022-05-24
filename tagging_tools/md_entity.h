#ifndef __MD_ENTITY_H__
#define __MD_ENTITY_H__

#include <string>
#include <vector>
#include "elf_loader.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void md_entity(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const elf_image_t& img, const std::vector<std::string>& yaml_files, reporter_t& err);
    
}

#endif // __MD_ENTITY_H__