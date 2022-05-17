#ifndef __MD_HEADER_H__
#define __MD_HEADER_H__

#include <list>
#include <string>
#include "reporter.h"

namespace policy_engine {

void md_header(const std::string& elf_filename, const std::string& soc_filename, const std::string& tag_filename, const std::string& policy_dir, std::list<std::string>& soc_exclude, reporter_t& err);

}

#endif // __MD_HEADER_H__