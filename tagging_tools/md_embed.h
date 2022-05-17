#ifndef __MD_EMBED_H__
#define __MD_EMBED_H__

#include <string>
#include "elf_loader.h"
#include "reporter.h"

namespace policy_engine {

int md_embed(const std::string& tag_filename, const std::string& policy_dir, elf_image_t& img, const std::string& elf_filename, reporter_t& err);
    
}

#endif // __MD_EMBED_H__