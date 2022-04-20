#ifndef __MD_ENTITY_H__
#define __MD_ENTITY_H__

#include <string>
#include <vector>
#include "basic_elf_io.h"

namespace policy_engine {

int md_entity(const std::string& policy_dir, const std::string& elf_file_name, const std::string& tag_file_name, const std::vector<std::string>& yaml_files, stdio_reporter_t& err, bool update=true);
    
}

#endif // __MD_ENTITY_H__