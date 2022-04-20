#ifndef __MD_INDEX_H__
#define __MD_INDEX_H__

#include <string>
#include "basic_elf_io.h"

namespace policy_engine {

int md_index(const std::string& tag_filename, const std::string& policy_dir, stdio_reporter_t& err);

}

#endif // __MD_INDEX_H__