#ifndef __ANNOTATE_H__
#define __ANNOTATE_H__

#include <string>
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void annotate_asm(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const std::string& asm_file, const std::string& output_file="");

}

#endif // __ANNOTATE_H__