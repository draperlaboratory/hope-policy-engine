#ifndef __MD_ENTITY_H__
#define __MD_ENTITY_H__

#include <string>
#include <vector>
#include "basic_elf_io.h"
#include "elf_loader.h"

namespace policy_engine {

int md_entity(const std::string& policy_dir, elf_image_t& img, const std::string& tag_file_name, const std::vector<std::string>& yaml_files, stdio_reporter_t& err, bool update=true);
    
}

#endif // __MD_ENTITY_H__