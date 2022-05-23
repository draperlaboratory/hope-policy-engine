#ifndef __MD_ASM_ANN_H__
#define __MD_ASM_ANN_H__

#include <string>
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void md_asm_ann(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const std::string& asm_file, const std::string& output_file="");

}

#endif // __MD_ASM_ANN_H__