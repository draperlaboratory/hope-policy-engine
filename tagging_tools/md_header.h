#ifndef __MD_HEADER_H__
#define __MD_HEADER_H__

#include <list>
#include <string>

namespace policy_engine {

int md_header(const std::string& elf_filename, const std::string& soc_filename, const std::string& tag_filename, const std::string& policy_dir, std::list<std::string>& soc_exclude, stdio_reporter_t& err);

}

#endif // __MD_HEADER_H__