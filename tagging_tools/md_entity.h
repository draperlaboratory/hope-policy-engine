#ifndef __MD_ENTITY_H__
#define __MD_ENTITY_H__

#include <string>
#include <vector>
#include "elf_loader.h"
#include "reporter.h"

namespace policy_engine {

void md_entity(const std::string& policy_dir, elf_image_t& img, const std::string& tag_file_name, const std::vector<std::string>& yaml_files, reporter_t& err, bool update=true);
    
}

#endif // __MD_ENTITY_H__