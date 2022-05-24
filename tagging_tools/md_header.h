#ifndef __MD_HEADER_H__
#define __MD_HEADER_H__

#include <list>
#include <string>
#include "elf_loader.h"
#include "metadata_factory.h"
#include "reporter.h"

namespace policy_engine {

void md_header(metadata_factory_t& factory, const elf_image_t& elf_image, const std::string& soc_filename, const std::string& tag_filename, const std::string& policy_dir, std::list<std::string>& soc_exclude, reporter_t& err);

}

#endif // __MD_HEADER_H__